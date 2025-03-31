#include <instance_communicator.hpp>

#include <logger.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/ts/buffer.hpp>
#include <boost/asio/ts/internet.hpp>

#include <iostream>

namespace instance_communicator
{
    bool SendSignal(const std::string& message, uint16_t port)
    {
        boost::asio::io_context ioContext;
        boost::asio::ip::tcp::socket socket(ioContext);
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

            // NOLINTNEXTLINE(bugprone-unused-return-value)
            socket.shutdown(boost::asio::socket_base::shutdown_type::shutdown_both, ec);

            std::cout << "Client sent signal: " << message << "\n";

            return true;
        }
        else
        {
            std::cout << "Client connect error: " << ec.message() << "\n";
            return false;
        }
    }

    InstanceCommunicator::InstanceCommunicator(uint16_t port, std::function<void(const std::string&)> handler)
        : m_port(port)
        , m_signalHandler(std::move(handler))
    {
    }

    void InstanceCommunicator::Stop()
    {
        m_keepRunning.store(false);
    }

    boost::asio::awaitable<void> InstanceCommunicator::Listen(
        boost::asio::io_context& ioContext) // NOLINT (cppcoreguidelines-avoid-reference-coroutine-parameters)
    {
        LogWarn("Setting InstanceCommunicator to listen on port {}", m_port);

        while (m_keepRunning.load())
        {
            boost::asio::ip::tcp::acceptor m_acceptor(
                ioContext, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), m_port));

            boost::asio::ip::tcp::socket socket(ioContext);
            boost::system::error_code ec;

            co_await m_acceptor.async_accept(socket, boost::asio::redirect_error(boost::asio::use_awaitable, ec));
            std::string message;
            if (!ec)
            {
                boost::asio::streambuf buffer;
                boost::asio::read_until(socket, buffer, "\n", ec);

                if (!ec || ec == boost::asio::error::eof)
                {
                    message =
                        std::string(boost::asio::buffers_begin(buffer.data()), boost::asio::buffers_end(buffer.data()));

                    message.replace(message.find('\n'), 1, ""); // remove newline added on send
                    LogWarn("Server received signal: {}", message);
                }
                else if (ec != boost::asio::error::eof)
                {
                    LogError("Listener read error: {}", ec.message());
                }
                boost::system::error_code ignored_ec;
                socket.close(ignored_ec); // NOLINT(bugprone-unused-return-value)
                if (m_signalHandler && !message.empty())
                {
                    m_signalHandler(message);
                }
            }
            else
            {
                LogError("Listener accept error: {}", ec.message());
            }
        }
        LogDebug("InstanceCommunicator stopping");
        co_return;
    }

} // namespace instance_communicator
