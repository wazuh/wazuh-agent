#include <instance_communicator.hpp>

#include <config.h>
#include <logger.hpp>

namespace instance_communicator
{
    boost::asio::awaitable<void> InstanceCommunicator::Listen(
        boost::asio::io_context& ioContext) // NOLINT (cppcoreguidelines-avoid-reference-coroutine-parameters)
    {
        const std::string socketPath = std::string(config::DEFAULT_RUN_PATH) + "/agent-socket";

        ::unlink(socketPath.c_str());

        boost::asio::local::stream_protocol::acceptor acceptor(ioContext);
        bool acceptorCreated = false;

        while (m_keepRunning.load() && !acceptorCreated)
        {
            try
            {
                acceptor.open();
                acceptor.bind(boost::asio::local::stream_protocol::endpoint(socketPath));
                acceptor.listen();
                ::chmod(socketPath.c_str(), 0660); // NOLINT (avoid-magic-numbers)
                acceptorCreated = true;
                LogDebug("InstanceCommunicator listening on {}", socketPath);
            }
            catch (const std::exception& e)
            {
                LogError("Failed to initialize acceptor: {}", e.what());
                acceptor.close();
                acceptorCreated = false;
            }
        }

        while (m_keepRunning.load())
        {
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

                    if (!ec || ec == boost::asio::error::eof)
                    {
                        std::string message(boost::asio::buffers_begin(buffer.data()),
                                            boost::asio::buffers_end(buffer.data()));

                        message.replace(message.find('\n'), 1, ""); // remove newline added on send

                        HandleSignal(message);
                    }
                    else
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

            co_await boost::asio::steady_timer(ioContext, std::chrono::seconds(1)) // NOLINT (avoid-magic-numbers)
                .async_wait(boost::asio::use_awaitable);
        }

        acceptor.close();
        ::unlink(socketPath.c_str());
        LogDebug("InstanceCommunicator stopping");
        co_return;
    }
} // namespace instance_communicator
