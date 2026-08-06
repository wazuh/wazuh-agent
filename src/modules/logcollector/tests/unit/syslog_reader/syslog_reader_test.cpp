#include <syslog_reader.hpp>

#include <boost/asio.hpp>
#include <gtest/gtest.h>

#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

using namespace logcollector;
using boost::asio::ip::tcp;
using boost::asio::ip::udp;

namespace
{
    constexpr auto LOOPBACK = "127.0.0.1";
    constexpr auto POLL_INTERVAL = std::chrono::milliseconds(20);
    constexpr int MAX_ATTEMPTS = 100;
    constexpr int EMPTY_DATAGRAM_ATTEMPTS = 10;
    constexpr unsigned short SAMPLE_PORT = 5514;
    constexpr auto BIND_ATTEMPT_WAIT = std::chrono::milliseconds(150);

    /// @brief Thread-safe sink that records messages pushed by a reader
    class MessageSink
    {
    public:
        void Push(const std::string& location, const std::string& log, const std::string& collectorType)
        {
            const std::lock_guard<std::mutex> lock(m_mutex);
            m_messages.emplace_back(location, log, collectorType);
        }

        std::size_t Count() const
        {
            const std::lock_guard<std::mutex> lock(m_mutex);
            return m_messages.size();
        }

        std::vector<std::tuple<std::string, std::string, std::string>> Messages() const
        {
            const std::lock_guard<std::mutex> lock(m_mutex);
            return m_messages;
        }

    private:
        mutable std::mutex m_mutex;
        std::vector<std::tuple<std::string, std::string, std::string>> m_messages;
    };

    unsigned short GetFreeUdpPort()
    {
        boost::asio::io_context io;
        udp::socket socket(io);
        socket.open(udp::v4());
        socket.bind(udp::endpoint(boost::asio::ip::make_address(LOOPBACK), 0));
        const auto port = socket.local_endpoint().port();
        socket.close();
        return port;
    }

    unsigned short GetFreeTcpPort()
    {
        boost::asio::io_context io;
        tcp::acceptor acceptor(io);
        acceptor.open(tcp::v4());
        acceptor.bind(tcp::endpoint(boost::asio::ip::make_address(LOOPBACK), 0));
        const auto port = acceptor.local_endpoint().port();
        acceptor.close();
        return port;
    }

    std::shared_ptr<SyslogReader> MakeReader(boost::asio::io_context& io,
                                             MessageSink& sink,
                                             SyslogProtocol protocol,
                                             unsigned short port)
    {
        auto push = [&sink](const std::string& location, const std::string& log, const std::string& collectorType)
        { sink.Push(location, log, collectorType); };

        // NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines)
        auto wait = [](std::chrono::milliseconds) -> Awaitable { co_return; };

        auto enqueue = [&io](boost::asio::awaitable<void> task)
        { boost::asio::co_spawn(io, std::move(task), boost::asio::detached); };

        return std::make_shared<SyslogReader>(
            push, wait, enqueue, protocol, std::string(LOOPBACK), static_cast<std::uint16_t>(port));
    }

    void WaitForCount(const MessageSink& sink, std::size_t expected)
    {
        for (int attempt = 0; attempt < MAX_ATTEMPTS && sink.Count() < expected; ++attempt)
        {
            std::this_thread::sleep_for(POLL_INTERVAL);
        }
    }

    void SendUdp(unsigned short port, const std::string& message)
    {
        boost::asio::io_context io;
        udp::socket socket(io);
        socket.open(udp::v4());
        socket.send_to(boost::asio::buffer(message), udp::endpoint(boost::asio::ip::make_address(LOOPBACK), port));
        socket.close();
    }

    bool SendTcp(unsigned short port, const std::string& message)
    {
        boost::asio::io_context io;
        tcp::socket socket(io);
        const tcp::endpoint endpoint(boost::asio::ip::make_address(LOOPBACK), port);

        for (int attempt = 0; attempt < MAX_ATTEMPTS; ++attempt)
        {
            boost::system::error_code ec;
            // NOLINTNEXTLINE(bugprone-unused-return-value)
            socket.connect(endpoint, ec);
            if (!ec)
            {
                boost::asio::write(socket, boost::asio::buffer(message), ec);
                socket.close();
                return !ec;
            }
            std::this_thread::sleep_for(POLL_INTERVAL);
        }
        return false;
    }
} // namespace

TEST(SyslogReader, ProtocolToString)
{
    EXPECT_EQ(SyslogReader::ProtocolToString(SyslogProtocol::Udp), "udp");
    EXPECT_EQ(SyslogReader::ProtocolToString(SyslogProtocol::Tcp), "tcp");
}

TEST(SyslogReader, ListenerIdFormat)
{
    auto sink = MessageSink();
    boost::asio::io_context io;
    auto reader = MakeReader(io, sink, SyslogProtocol::Udp, SAMPLE_PORT);
    EXPECT_EQ(reader->ListenerId(), "udp:127.0.0.1:5514");
}

TEST(SyslogReader, UdpReceivesMessage)
{
    const auto port = GetFreeUdpPort();
    MessageSink sink;
    boost::asio::io_context io;
    auto reader = MakeReader(io, sink, SyslogProtocol::Udp, port);

    boost::asio::co_spawn(io, reader->Run(), boost::asio::detached);
    std::thread ioThread([&io]() { io.run(); });

    // Resend until the listener is bound and the datagram is delivered.
    for (int attempt = 0; attempt < MAX_ATTEMPTS && sink.Count() == 0; ++attempt)
    {
        SendUdp(port, "<13>Jun 25 10:00:00 testhost testapp: UDP listener test\n");
        std::this_thread::sleep_for(POLL_INTERVAL);
    }

    reader->Stop();
    ioThread.join();

    ASSERT_GE(sink.Count(), 1u);
    const auto messages = sink.Messages();
    EXPECT_EQ(std::get<1>(messages[0]), "<13>Jun 25 10:00:00 testhost testapp: UDP listener test");
    EXPECT_EQ(std::get<2>(messages[0]), REMOTE_SYSLOG_READER_TYPE);
    EXPECT_EQ(std::get<0>(messages[0]), reader->ListenerId());
}

TEST(SyslogReader, UdpEmptyDatagramIsIgnored)
{
    const auto port = GetFreeUdpPort();
    MessageSink sink;
    boost::asio::io_context io;
    auto reader = MakeReader(io, sink, SyslogProtocol::Udp, port);

    boost::asio::co_spawn(io, reader->Run(), boost::asio::detached);
    std::thread ioThread([&io]() { io.run(); });

    for (int attempt = 0; attempt < EMPTY_DATAGRAM_ATTEMPTS; ++attempt)
    {
        SendUdp(port, "\n");
        std::this_thread::sleep_for(POLL_INTERVAL);
    }

    reader->Stop();
    ioThread.join();

    EXPECT_EQ(sink.Count(), 0u);
}

TEST(SyslogReader, TcpReceivesMessage)
{
    const auto port = GetFreeTcpPort();
    MessageSink sink;
    boost::asio::io_context io;
    auto reader = MakeReader(io, sink, SyslogProtocol::Tcp, port);

    boost::asio::co_spawn(io, reader->Run(), boost::asio::detached);
    std::thread ioThread([&io]() { io.run(); });

    const bool sent = SendTcp(port, "<13>Jun 25 10:00:00 testhost testapp: TCP listener test\n");
    WaitForCount(sink, 1);

    reader->Stop();
    ioThread.join();

    ASSERT_TRUE(sent);
    ASSERT_GE(sink.Count(), 1u);
    const auto messages = sink.Messages();
    EXPECT_EQ(std::get<1>(messages[0]), "<13>Jun 25 10:00:00 testhost testapp: TCP listener test");
    EXPECT_EQ(std::get<2>(messages[0]), REMOTE_SYSLOG_READER_TYPE);
}

TEST(SyslogReader, TcpHandlesMultipleLines)
{
    const auto port = GetFreeTcpPort();
    MessageSink sink;
    boost::asio::io_context io;
    auto reader = MakeReader(io, sink, SyslogProtocol::Tcp, port);

    boost::asio::co_spawn(io, reader->Run(), boost::asio::detached);
    std::thread ioThread([&io]() { io.run(); });

    const bool sent = SendTcp(port, "first message\nsecond message\nthird message\n");
    WaitForCount(sink, 3);

    reader->Stop();
    ioThread.join();

    ASSERT_TRUE(sent);
    EXPECT_GE(sink.Count(), 3u);
}

TEST(SyslogReader, MultipleListenersReceive)
{
    const auto udpPort = GetFreeUdpPort();
    const auto tcpPort = GetFreeTcpPort();
    MessageSink sink;
    boost::asio::io_context io;

    auto udpReader = MakeReader(io, sink, SyslogProtocol::Udp, udpPort);
    auto tcpReader = MakeReader(io, sink, SyslogProtocol::Tcp, tcpPort);

    boost::asio::co_spawn(io, udpReader->Run(), boost::asio::detached);
    boost::asio::co_spawn(io, tcpReader->Run(), boost::asio::detached);
    std::thread ioThread([&io]() { io.run(); });

    const bool tcpSent = SendTcp(tcpPort, "tcp listener message\n");
    for (int attempt = 0; attempt < MAX_ATTEMPTS && sink.Count() < 2; ++attempt)
    {
        SendUdp(udpPort, "udp listener message\n");
        std::this_thread::sleep_for(POLL_INTERVAL);
    }

    udpReader->Stop();
    tcpReader->Stop();
    ioThread.join();

    ASSERT_TRUE(tcpSent);
    EXPECT_GE(sink.Count(), 2u);
}

// Simulates a port already taken by another service (e.g. rsyslog): the listener
// must fail to bind, log the error, not start, and must not crash or hang the agent.
TEST(SyslogReader, TcpBindFailsWhenPortInUse)
{
    const auto port = GetFreeTcpPort();

    // Occupy the port first, as another process (rsyslog/syslog-ng) would.
    boost::asio::io_context occupierIo;
    tcp::acceptor occupier(occupierIo);
    occupier.open(tcp::v4());
    occupier.set_option(boost::asio::socket_base::reuse_address(true));
    boost::system::error_code bindEc;
    // NOLINTNEXTLINE(bugprone-unused-return-value)
    occupier.bind(tcp::endpoint(boost::asio::ip::make_address(LOOPBACK), port), bindEc);
    ASSERT_FALSE(bindEc) << "precondition: occupier must bind the port";
    occupier.listen();

    MessageSink sink;
    boost::asio::io_context io;
    auto reader = MakeReader(io, sink, SyslogProtocol::Tcp, port);

    boost::asio::co_spawn(io, reader->Run(), boost::asio::detached);
    std::thread ioThread([&io]() { io.run(); });

    std::this_thread::sleep_for(BIND_ATTEMPT_WAIT); // let the reader attempt (and fail) the bind
    reader->Stop();
    ioThread.join(); // must not hang: the reader fails to bind and returns
    occupier.close();

    EXPECT_EQ(sink.Count(), 0u); // listener never came up, so nothing was ingested
}

TEST(SyslogReader, UdpBindFailsWhenPortInUse)
{
    const auto port = GetFreeUdpPort();

    boost::asio::io_context occupierIo;
    udp::socket occupier(occupierIo);
    occupier.open(udp::v4());
    occupier.set_option(boost::asio::socket_base::reuse_address(true));
    boost::system::error_code bindEc;
    // NOLINTNEXTLINE(bugprone-unused-return-value)
    occupier.bind(udp::endpoint(boost::asio::ip::make_address(LOOPBACK), port), bindEc);
    ASSERT_FALSE(bindEc) << "precondition: occupier must bind the port";

    MessageSink sink;
    boost::asio::io_context io;
    auto reader = MakeReader(io, sink, SyslogProtocol::Udp, port);

    boost::asio::co_spawn(io, reader->Run(), boost::asio::detached);
    std::thread ioThread([&io]() { io.run(); });

    std::this_thread::sleep_for(BIND_ATTEMPT_WAIT); // let the reader attempt (and fail) the bind
    reader->Stop();
    ioThread.join();
    occupier.close();

    EXPECT_EQ(sink.Count(), 0u);
}
