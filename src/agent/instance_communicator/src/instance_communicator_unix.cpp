#include <instance_communicator.hpp>

#include <socket_wrapper_unix.hpp>

#include <logger.hpp>

#include <fmt/format.h>

namespace instance_communicator
{
    boost::asio::awaitable<void> InstanceCommunicator::Listen(
        const std::string& runPath, // NOLINT (cppcoreguidelines-avoid-reference-coroutine-parameters)
        std::unique_ptr<ISocketWrapper> socketWrapper,
        [[maybe_unused]] std::unique_ptr<IPipeWrapper> pipeWrapper)
    {
        constexpr int A_SECOND_IN_MILLIS = 1000;
        constexpr mode_t SOCKET_FILE_PERMISSIONS = 0660;
        auto executor = co_await boost::asio::this_coro::executor;

        if (socketWrapper == nullptr)
        {
            socketWrapper = std::make_unique<SocketWrapper>(executor);
        }

        const std::string socketPath = fmt::format("{}/agent-socket", runPath);

        ::unlink(socketPath.c_str());

        while (m_keepRunning.load())
        {
            try
            {
                socketWrapper->AcceptorOpen();
                socketWrapper->AcceptorBind(boost::asio::local::stream_protocol::endpoint(socketPath));
                socketWrapper->AcceptorListen();

                ::chmod(socketPath.c_str(), SOCKET_FILE_PERMISSIONS);

                LogDebug("InstanceCommunicator listening on {}", socketPath);
                break;
            }
            catch (const std::exception& e)
            {
                socketWrapper->AcceptorClose();

                LogError("Failed to initialize acceptor: {}", e.what());
            }

            boost::asio::steady_timer timer(executor, std::chrono::milliseconds(A_SECOND_IN_MILLIS));
            co_await timer.async_wait(boost::asio::use_awaitable);
        }

        while (m_keepRunning.load())
        {
            boost::system::error_code ec;
            try
            {
                co_await socketWrapper->AcceptorAsyncAccept(ec);

                if (!ec)
                {
                    boost::asio::streambuf buffer;

                    co_await socketWrapper->SocketReadUntil(buffer, ec);

                    if (!ec || ec == boost::asio::error::eof)
                    {
                        std::string message(boost::asio::buffers_begin(buffer.data()),
                                            boost::asio::buffers_end(buffer.data()));

                        if (auto pos = message.find('\n'); pos != std::string::npos)
                        {
                            message.erase(pos, 1);
                        }

                        LogDebug("Received signal: {}", message);

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

                socketWrapper->SocketShutdown(ec);
            }
            catch (const std::exception& e)
            {
                LogError("Listener exception: {}", e.what());

                socketWrapper->SocketShutdown(ec);
            }

            boost::asio::steady_timer timer(executor, std::chrono::milliseconds(A_SECOND_IN_MILLIS));
            co_await timer.async_wait(boost::asio::use_awaitable);
        }

        socketWrapper->AcceptorClose();

        ::unlink(socketPath.c_str());

        LogDebug("InstanceCommunicator stopping");
        co_return;
    }
} // namespace instance_communicator
