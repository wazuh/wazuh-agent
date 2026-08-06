#include "syslog_reader.hpp"

#include <logger.hpp>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <istream>
#include <utility>
#include <vector>

using namespace logcollector;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;

namespace
{
    /// @brief Removes a trailing carriage return / line feed sequence from a message
    void TrimLineEnding(std::string& message)
    {
        while (!message.empty() && (message.back() == '\n' || message.back() == '\r'))
        {
            message.pop_back();
        }
    }
} // namespace

SyslogReader::SyslogReader(
    std::function<void(const std::string& location, const std::string& log, const std::string& collectorType)>
        pushMessageFunc,
    std::function<Awaitable(std::chrono::milliseconds)> waitFunc,
    std::function<void(boost::asio::awaitable<void>)> enqueueTaskFunc,
    SyslogProtocol protocol,
    std::string bindAddress,
    std::uint16_t port)
    : IReader(std::move(pushMessageFunc), std::move(waitFunc))
    , m_enqueueTask(std::move(enqueueTaskFunc))
    , m_protocol(protocol)
    , m_bindAddress(std::move(bindAddress))
    , m_port(port)
{
}

std::string SyslogReader::ProtocolToString(SyslogProtocol protocol)
{
    return protocol == SyslogProtocol::Udp ? "udp" : "tcp";
}

std::string SyslogReader::ListenerId() const
{
    return ProtocolToString(m_protocol) + ":" + m_bindAddress + ":" + std::to_string(m_port);
}

Awaitable SyslogReader::Run()
{
    const auto executor = co_await boost::asio::this_coro::executor;

    {
        const std::lock_guard<std::mutex> lock(m_socketMutex);
        m_executor = executor;
    }

    if (!m_keepRunning.load())
    {
        co_return;
    }

    if (m_protocol == SyslogProtocol::Udp)
    {
        co_await RunUdp();
    }
    else
    {
        co_await RunTcp();
    }
}

void SyslogReader::Stop()
{
    m_keepRunning.store(false);

    const std::lock_guard<std::mutex> lock(m_socketMutex);

    if (!m_executor.has_value())
    {
        return;
    }

    // Sockets are not thread-safe: close them on the io_context thread.
    boost::asio::post(*m_executor,
                      [this]()
                      {
                          const std::lock_guard<std::mutex> innerLock(m_socketMutex);
                          boost::system::error_code ec;

                          // NOLINTBEGIN(bugprone-unused-return-value)
                          if (m_udpSocket)
                          {
                              m_udpSocket->close(ec);
                          }

                          if (m_acceptor)
                          {
                              m_acceptor->close(ec);
                          }

                          for (const auto& client : m_clients)
                          {
                              if (auto socket = client.lock())
                              {
                                  socket->close(ec);
                              }
                          }
                          // NOLINTEND(bugprone-unused-return-value)
                      });
}

Awaitable SyslogReader::RunUdp()
{
    const auto executor = co_await boost::asio::this_coro::executor;
    boost::system::error_code ec;

    const auto address = boost::asio::ip::make_address(m_bindAddress, ec);
    if (ec)
    {
        LogError("Invalid agent-side syslog listener configuration: invalid bind address {}", m_bindAddress);
        co_return;
    }

    const auto socket = std::make_shared<udp::socket>(executor);
    const udp::endpoint endpoint(address, m_port);

    // NOLINTNEXTLINE(bugprone-unused-return-value)
    socket->open(endpoint.protocol(), ec);
    if (!ec)
    {
        // SO_REUSEADDR is intentionally not set for UDP: on some platforms it allows
        // a second socket to bind an address:port already in use, which would let the
        // listener start silently alongside another service (e.g. rsyslog) and split
        // datagrams unpredictably. Without it, a busy port fails to bind and is reported.
        // NOLINTNEXTLINE(bugprone-unused-return-value)
        socket->bind(endpoint, ec);
    }

    if (ec)
    {
        LogError("Failed to start agent-side syslog UDP listener on {}:{}: {}", m_bindAddress, m_port, ec.message());
        co_return;
    }

    {
        const std::lock_guard<std::mutex> lock(m_socketMutex);
        m_udpSocket = socket;
    }

    if (!m_keepRunning.load())
    {
        // NOLINTNEXTLINE(bugprone-unused-return-value)
        socket->close(ec);
        co_return;
    }

    LogInfo("Started agent-side syslog UDP listener on {}:{}", m_bindAddress, m_port);

    std::vector<char> buffer(MAX_MESSAGE_SIZE);
    udp::endpoint sender;

    while (m_keepRunning.load())
    {
        const std::size_t bytes = co_await socket->async_receive_from(
            boost::asio::buffer(buffer), sender, boost::asio::redirect_error(boost::asio::use_awaitable, ec));

        if (ec)
        {
            if (ec == boost::asio::error::operation_aborted)
            {
                break;
            }

            if (ec == boost::asio::error::message_size)
            {
                LogDebug("Dropped oversized syslog datagram from {} on {}", sender.address().to_string(), ListenerId());
                continue;
            }

            LogDebug("Syslog UDP listener {} receive error: {}", ListenerId(), ec.message());
            continue;
        }

        LogTrace("Received syslog UDP datagram from {} on {}", sender.address().to_string(), ListenerId());
        ProcessMessage(std::string(buffer.data(), bytes));
    }

    LogInfo("Stopped agent-side syslog UDP listener on {}:{}", m_bindAddress, m_port);
}

Awaitable SyslogReader::RunTcp()
{
    const auto executor = co_await boost::asio::this_coro::executor;
    boost::system::error_code ec;

    const auto address = boost::asio::ip::make_address(m_bindAddress, ec);
    if (ec)
    {
        LogError("Invalid agent-side syslog listener configuration: invalid bind address {}", m_bindAddress);
        co_return;
    }

    const auto acceptor = std::make_shared<tcp::acceptor>(executor);
    const tcp::endpoint endpoint(address, m_port);

    // NOLINTBEGIN(bugprone-unused-return-value)
    acceptor->open(endpoint.protocol(), ec);
    if (!ec)
    {
        acceptor->set_option(boost::asio::socket_base::reuse_address(true), ec);
    }
    if (!ec)
    {
        acceptor->bind(endpoint, ec);
    }
    if (!ec)
    {
        acceptor->listen(boost::asio::socket_base::max_listen_connections, ec);
    }
    // NOLINTEND(bugprone-unused-return-value)

    if (ec)
    {
        LogError("Failed to start agent-side syslog TCP listener on {}:{}: {}", m_bindAddress, m_port, ec.message());
        co_return;
    }

    {
        const std::lock_guard<std::mutex> lock(m_socketMutex);
        m_acceptor = acceptor;
    }

    if (!m_keepRunning.load())
    {
        // NOLINTNEXTLINE(bugprone-unused-return-value)
        acceptor->close(ec);
        co_return;
    }

    LogInfo("Started agent-side syslog TCP listener on {}:{}", m_bindAddress, m_port);

    while (m_keepRunning.load())
    {
        const auto socket = std::make_shared<tcp::socket>(executor);

        co_await acceptor->async_accept(*socket, boost::asio::redirect_error(boost::asio::use_awaitable, ec));

        if (ec)
        {
            if (ec == boost::asio::error::operation_aborted)
            {
                break;
            }

            LogDebug("Syslog TCP listener {} accept error: {}", ListenerId(), ec.message());
            continue;
        }

        boost::system::error_code remoteEc;
        const auto remoteEndpoint = socket->remote_endpoint(remoteEc);
        const std::string remote =
            remoteEc ? std::string("unknown")
                     : remoteEndpoint.address().to_string() + ":" + std::to_string(remoteEndpoint.port());

        {
            const std::lock_guard<std::mutex> lock(m_socketMutex);
            m_clients.remove_if([](const std::weak_ptr<tcp::socket>& client) { return client.expired(); });
            m_clients.push_back(socket);
        }

        LogDebug("Accepted syslog TCP client {} on {}", remote, ListenerId());
        m_enqueueTask(HandleTcpClient(socket, remote));
    }

    LogInfo("Stopped agent-side syslog TCP listener on {}:{}", m_bindAddress, m_port);
}

// Parameters are passed by value on purpose: a coroutine must own them so they
// remain valid across suspension points.
// NOLINTNEXTLINE(performance-unnecessary-value-param)
Awaitable SyslogReader::HandleTcpClient(std::shared_ptr<tcp::socket> socket, std::string remote)
{
    boost::asio::streambuf buffer(MAX_MESSAGE_SIZE);
    boost::system::error_code ec;

    while (m_keepRunning.load())
    {
        co_await boost::asio::async_read_until(
            *socket, buffer, '\n', boost::asio::redirect_error(boost::asio::use_awaitable, ec));

        if (ec)
        {
            if (ec == boost::asio::error::not_found)
            {
                // Buffer filled without a line delimiter: the message is too large.
                LogDebug("Dropped oversized syslog message from {} on {}", remote, ListenerId());
            }
            else if (ec != boost::asio::error::eof && ec != boost::asio::error::operation_aborted)
            {
                LogDebug("Syslog TCP client {} read error on {}: {}", remote, ListenerId(), ec.message());
            }
            break;
        }

        std::istream stream(&buffer);
        std::string line;
        std::getline(stream, line);
        ProcessMessage(std::move(line));
    }

    boost::system::error_code closeEc;
    // NOLINTNEXTLINE(bugprone-unused-return-value)
    socket->close(closeEc);

    LogDebug("Closed syslog TCP client {} on {}", remote, ListenerId());
}

void SyslogReader::ProcessMessage(std::string message) const
{
    TrimLineEnding(message);

    if (message.empty())
    {
        return;
    }

    if (message.size() > MAX_MESSAGE_SIZE)
    {
        LogDebug("Dropped oversized syslog message on {}", ListenerId());
        return;
    }

    m_pushMessage(ListenerId(), message, m_collectorType);
}
