#include <ilistener_wrapper.hpp>
#include <instance_communicator.hpp>
#include <listener_wrapper.hpp>

#include <boost/asio.hpp>
#include <boost/system/error_code.hpp>

#include <logger.hpp>

#include <array>
#include <chrono>
#include <memory>
#include <string>

#ifdef USE_PIPE_WRAPPER
#include <ipipe_wrapper.hpp>
using ListenerType = instance_communicator::ListenerWrapper<instance_communicator::IPipeWrapper>;
#else
#include <isocket_wrapper.hpp>
using ListenerType = instance_communicator::ListenerWrapper<instance_communicator::ISocketWrapper>;
#endif

namespace instance_communicator
{
    InstanceCommunicator::InstanceCommunicator(
        std::function<void(const std::optional<std::string>&)> reloadModulesHandler)
        : m_reloadModulesHandler(std::move(reloadModulesHandler))
    {
        if (m_reloadModulesHandler == nullptr)
        {
            throw std::runtime_error(std::string("Invalid handler passed."));
        }
    }

    void InstanceCommunicator::HandleSignal(const std::string& signal) const
    {
        if (signal == "RELOAD")
        {
            m_reloadModulesHandler(std::nullopt);
        }
        else if (signal.starts_with("RELOAD-MODULE:"))
        {
            m_reloadModulesHandler(signal.substr(signal.find(':') + 1));
        }
        else
        {
            LogWarn("Invalid message received from CLI: {}", signal);
        }
    }

    boost::asio::awaitable<void> InstanceCommunicator::Listen(
        const std::string& runPath, // NOLINT(cppcoreguidelines-avoid-reference-coroutine-parameters)
        std::unique_ptr<IListenerWrapper> listenerWrapper)
    {
        constexpr int A_SECOND_IN_MILLIS = 1000;
        constexpr std::size_t BUFFER_SIZE = 4096;
        auto executor = co_await boost::asio::this_coro::executor;

        if (listenerWrapper == nullptr)
        {
            listenerWrapper = std::make_unique<ListenerType>(executor);
        }

        while (m_keepRunning.load())
        {
            if (!listenerWrapper->CreateOrOpen(runPath, BUFFER_SIZE))
            {
                LogError("Failed to create or open listener");
            }
            else
            {
                LogDebug("InstanceCommunicator listening");

                boost::system::error_code ec;
                try
                {
                    co_await listenerWrapper->AsyncAccept(ec);

                    if (!ec)
                    {
                        std::array<char, BUFFER_SIZE> buffer {};

                        std::size_t bytes = co_await listenerWrapper->AsyncRead(buffer.data(), buffer.size(), ec);

                        if (!ec || ec == boost::asio::error::eof)
                        {
                            std::string message(buffer.data(), bytes);

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
                }
                catch (const std::exception& e)
                {
                    LogError("Listener exception: {}", e.what());
                }

                listenerWrapper->Close();
            }

            boost::asio::steady_timer timer(executor, std::chrono::milliseconds(A_SECOND_IN_MILLIS));
            co_await timer.async_wait(boost::asio::use_awaitable);
        }

        LogDebug("InstanceCommunicator stopping");
        co_return;
    }

    void InstanceCommunicator::Stop()
    {
        m_keepRunning.store(false);
    }
} // namespace instance_communicator
