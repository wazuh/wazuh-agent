#include <instance_communicator.hpp>

#include <logger.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

#include <iostream>

namespace instance_communicator
{
    bool SendSignal(const std::string& message)
    {
        boost::asio::io_context ioContext;
        boost::asio::local::stream_protocol::socket socket(ioContext);
        boost::system::error_code ec;

        // NOLINTNEXTLINE(bugprone-unused-return-value)
        socket.connect(boost::asio::local::stream_protocol::endpoint(socketPath), ec);

        if (!ec)
        {
            std::string command = message + "\n"; // Add newline for read_until
            boost::asio::write(socket, boost::asio::buffer(command), ec);
            if (ec)
            {
                std::cout << "Client write error: " << ec.message() << "\n";
                return false;
            }

            // NOLINTBEGIN(bugprone-unused-return-value)
            socket.shutdown(boost::asio::socket_base::shutdown_type::shutdown_both, ec);
            socket.close(ec);
            // NOLINTEND(bugprone-unused-return-value)

            std::cout << "Client sent signal: " << message << "\n";
            return true;
        }
        else
        {
            std::cout << "Client connect error: " << ec.message() << "\n";
            return false;
        }
    }

    // bool SendSignal(const std::string& message, uint16_t port)
    // {
    //     boost::asio::io_context ioContext;
    //     boost::asio::ip::tcp::socket socket(ioContext);
    //     boost::system::error_code ec;

    //     // NOLINTNEXTLINE(bugprone-unused-return-value)
    //     socket.connect(boost::asio::ip::tcp::endpoint(boost::asio::ip::address::from_string("127.0.0.1"), port), ec);

    //     if (!ec)
    //     {
    //         std::string signal_message = message + "\n"; // Add newline for read_until
    //         boost::asio::write(socket, boost::asio::buffer(signal_message), ec);
    //         if (ec)
    //         {
    //             std::cout << "Client write error: " << ec.message() << "\n";
    //             return false;
    //         }

    //         // NOLINTNEXTLINE(bugprone-unused-return-value)
    //         socket.shutdown(boost::asio::socket_base::shutdown_type::shutdown_both, ec);

    //         std::cout << "Client sent signal: " << message << "\n";

    //         return true;
    //     }
    //     else
    //     {
    //         std::cout << "Client connect error: " << ec.message() << "\n";
    //         return false;
    //     }
    // }

    InstanceCommunicator::InstanceCommunicator(std::function<void(const std::string&)> handler)
        : m_signalHandler(std::move(handler))
    {
    }

    void InstanceCommunicator::Stop()
    {
        m_keepRunning.store(false);
    }

    boost::asio::awaitable<void> InstanceCommunicator::Listen(
        boost::asio::io_context& ioContext) // NOLINT (cppcoreguidelines-avoid-reference-coroutine-parameters)
    {
        const std::string socket_path = "/tmp/agent-socket";
        LogCritical("InstanceCommunicator to listen on {}", socket_path);

        ::unlink(socket_path.c_str());

        boost::asio::local::stream_protocol::acceptor acceptor(ioContext);
        try
        {
            acceptor.open();
            acceptor.bind(boost::asio::local::stream_protocol::endpoint(socket_path));
            acceptor.listen();
            ::chmod(socket_path.c_str(), 0666); // NOLINT (avoid-magic-numbers)
        }
        catch (const std::exception& e)
        {
            LogError("Failed to initialize acceptor: {}", e.what());
            co_return;
        }

        while (m_keepRunning.load())
        {
            bool delayRetry = true;

            try
            {
                boost::asio::local::stream_protocol::socket socket(ioContext);
                boost::system::error_code ec;

                co_await acceptor.async_accept(socket, boost::asio::redirect_error(boost::asio::use_awaitable, ec));

                if (!ec)
                {
                    boost::asio::streambuf buffer;

                    co_await boost::asio::async_read_until(
                        socket, buffer, "\n", boost::asio::redirect_error(boost::asio::use_awaitable, ec));

                    socket.shutdown(boost::asio::socket_base::shutdown_type::shutdown_both);
                    socket.close();

                    if (!ec || ec == boost::asio::error::eof)
                    {
                        delayRetry = false;

                        std::string message(boost::asio::buffers_begin(buffer.data()),
                                            boost::asio::buffers_end(buffer.data()));

                        message.replace(message.find('\n'), 1, ""); // remove newline added on send

                        LogWarn("Server received signal: {}", message);

                        if (m_signalHandler && !message.empty())
                        {
                            m_signalHandler(message);
                        }
                    }
                    else if (ec != boost::asio::error::eof)
                    {
                        LogError("Listener read error: {}", ec.message());
                    }
                }
                else
                {
                    LogError("Listener accept error: {}", ec.message());
                }
            }
            catch (const std::exception& e)
            {
                LogError("Listener exception: {}", e.what());
            }

            if (delayRetry)
            {
                co_await boost::asio::steady_timer(ioContext, std::chrono::seconds(1)) // NOLINT (avoid-magic-numbers)
                    .async_wait(boost::asio::use_awaitable);
            }
        }

        acceptor.close();
        ::unlink(socket_path.c_str());
        LogDebug("InstanceCommunicator stopping");
        co_return;
    }

    // boost::asio::awaitable<void> InstanceCommunicator::Listen(
    //     boost::asio::io_context& ioContext) // NOLINT (cppcoreguidelines-avoid-reference-coroutine-parameters)
    // {
    //     LogWarn("Setting InstanceCommunicator to listen on port {}", m_port);

    //     while (m_keepRunning.load())
    //     {
    //         boost::asio::ip::tcp::acceptor m_acceptor(
    //             ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), m_port));

    //         boost::asio::ip::tcp::socket socket(ioContext);
    //         boost::system::error_code ec;

    //         co_await m_acceptor.async_accept(socket, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
    //         std::string message;
    //         if (!ec)
    //         {
    //             boost::asio::streambuf buffer;
    //             boost::asio::read_until(socket, buffer, "\n", ec);

    //             if (!ec || ec == boost::asio::error::eof)
    //             {
    //                 message =
    //                     std::string(boost::asio::buffers_begin(buffer.data()),
    //                     boost::asio::buffers_end(buffer.data()));

    //                 message.replace(message.find('\n'), 1, ""); // remove newline added on send
    //                 LogWarn("Server received signal: {}", message);
    //             }
    //             else if (ec != boost::asio::error::eof)
    //             {
    //                 LogError("Listener read error: {}", ec.message());
    //             }

    //             socket.close(ec); // NOLINT(bugprone-unused-return-value)

    //             if (m_signalHandler && !message.empty())
    //             {
    //                 m_signalHandler(message);
    //             }
    //         }
    //         else
    //         {
    //             LogError("Listener accept error: {}", ec.message());
    //         }
    //     }
    //     LogDebug("InstanceCommunicator stopping");
    //     co_return;
    // }

} // namespace instance_communicator
