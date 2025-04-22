#include <instance_communicator.hpp>

#include <pipe_wrapper_win.hpp>

#include <logger.hpp>

namespace instance_communicator
{
    boost::asio::awaitable<void> InstanceCommunicator::Listen(
        [[maybe_unused]] const std::string& runPath, // NOLINT (cppcoreguidelines-avoid-reference-coroutine-parameters)
        [[maybe_unused]] std::unique_ptr<ISocketWrapper> socketWrapper,
        std::unique_ptr<IPipeWrapper> pipeWrapper)
    {
        constexpr int A_SECOND_IN_MILLIS = 1000;
        constexpr int BUFFER_SIZE = 4096;
        auto executor = co_await boost::asio::this_coro::executor;

        if (pipeWrapper == nullptr)
        {
            pipeWrapper = std::make_unique<PipeWrapper>(executor);
        }

        const std::string pipeName = "\\\\.\\pipe\\agent-pipe";

        while (m_keepRunning.load())
        {
            if (!pipeWrapper->PipeCreate(pipeName, BUFFER_SIZE))
            {
                LogError("Failed to create pipe");
            }
            else
            {
                LogDebug("InstanceCommunicator listening on {}", pipeName);

                boost::system::error_code ec;
                try
                {
                    co_await pipeWrapper->PipeAsyncConnect(ec);

                    if (!ec)
                    {
                        std::array<char, BUFFER_SIZE> buffer {};

                        std::size_t n = co_await pipeWrapper->PipeAsyncRead(buffer.data(), buffer.size(), ec);

                        if (!ec || ec == boost::asio::error::eof)
                        {
                            std::string message(buffer.data(), n);

                            if (auto pos = message.find('\n'); pos != std::string::npos)
                            {
                                message.erase(pos, 1);
                            }

                            LogDebug("Received signal: {}", message);

                            HandleSignal(message);
                        }
                        else
                        {
                            LogError("Listener read error (code {}): {}", ec.value(), ec.message());
                        }
                    }
                    else
                    {
                        LogError("Listener connect error (code {}): {}", ec.value(), ec.message());
                    }

                    pipeWrapper->PipeClose();
                }
                catch (const std::exception& e)
                {
                    LogError("Listener exception: {}", e.what());

                    pipeWrapper->PipeClose();
                }
            }

            boost::asio::steady_timer timer(executor, std::chrono::milliseconds(A_SECOND_IN_MILLIS));
            co_await timer.async_wait(boost::asio::use_awaitable);
        }

        LogDebug("InstanceCommunicator stopping");
        co_return;
    }
} // namespace instance_communicator
