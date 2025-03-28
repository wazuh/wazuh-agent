#include <instance_communicator.hpp>

#include <logger.hpp>

#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

#include <atomic>
#include <future>
#include <iostream>

namespace instance_communicator
{

    InstanceCommunicator::InstanceCommunicator(boost::asio::io_context& ioContext,
                                               uint16_t port,
                                               std::function<void(const std::string&)> handler)
        : m_ioContext(ioContext)
        , m_port(port)
        , m_acceptor(m_ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), m_port))
        , m_signaled(false)
        , m_signalHandler(std::move(handler))
    {
    }

    std::optional<std::string> InstanceCommunicator::WaitForSignal()
    {
        std::promise<std::optional<std::string>> signal_promise;
        std::future<std::optional<std::string>> signal_future = signal_promise.get_future();

        boost::asio::ip::tcp::socket socket(m_ioContext);
        boost::system::error_code ec;
        m_acceptor.accept(socket, ec); // NOLINT(bugprone-unused-return-value)

        if (!ec)
        {
            boost::asio::streambuf buffer;
            boost::asio::read_until(socket, buffer, "\n", ec);

            if (!ec || ec == boost::asio::error::eof)
            {
                std::string message(boost::asio::buffers_begin(buffer.data()), boost::asio::buffers_end(buffer.data()));
                signal_promise.set_value(message.empty() ? std::optional<std::string>(std::nullopt)
                                                         : std::optional<std::string>(message));
                LogDebug("Server received signal: {}", message);
            }
            else if (ec != boost::asio::error::eof)
            {
                LogError("Listener read error: {}", ec.message());
                signal_promise.set_value(std::nullopt);
            }
            boost::system::error_code ignored_ec;
            socket.close(ignored_ec); // NOLINT(bugprone-unused-return-value)
        }
        else
        {
            LogError("Listener accept error: {}", ec.message());
            signal_promise.set_value(std::nullopt);
        }

        return signal_future.get();
    }

    void InstanceCommunicator::Listen()
    {
        LogDebug("Server listening on port {}", m_port);

        while (!m_signaled)
        {
            std::optional<std::string> received_message = WaitForSignal(); // Blocks until a connection
            if (received_message.has_value())
            {
                m_signaled = true;
                if (m_signalHandler)
                {
                    m_signalHandler(received_message.value());
                }
            }
        }
        std::cout << "Server shutting down after receiving signal.\n";
    }

    bool SendSignal(const std::string& message, uint16_t port)
    {
        boost::asio::io_context client_io_context;
        boost::asio::ip::tcp::socket socket(client_io_context);
        boost::system::error_code ec;

        // NOLINTNEXTLINE(bugprone-unused-return-value)
        socket.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), port), ec);

        if (!ec)
        {
            std::string signal_message = message + "\n"; // Add newline for read_until
            boost::asio::write(socket, boost::asio::buffer(signal_message), ec);
            if (ec)
            {
                std::cout << "Client write error: " << ec.message() << "\n";
                return false;
            }
            boost::system::error_code ignored_ec;
            socket.close(ignored_ec); // NOLINT(bugprone-unused-return-value)
            std::cout << "Client sent signal: " << message << "\n";
            return true;
        }
        else
        {
            std::cout << "Client connect error: " << ec.message() << "\n";
            return false;
        }
    }
} // namespace instance_communicator
