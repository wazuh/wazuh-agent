#pragma once

#include <command_store.hpp>
#include <logger.hpp>

#include <boost/asio.hpp>

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>

namespace command_handler
{
    class CommandHandler
    {
    public:
        template<typename T>
        boost::asio::awaitable<void> CommandsProcessingTask(
            const std::function<std::optional<T>()> GetCommandFromQueue,
            const std::function<void()> PopCommandFromQueue,
            const std::function<boost::asio::awaitable<module_command::CommandExecutionResult>(T&)> DispatchCommand)
        {
            using namespace std::chrono_literals;
            const auto executor = co_await boost::asio::this_coro::executor;
            std::unique_ptr<boost::asio::steady_timer> expTimer = std::make_unique<boost::asio::steady_timer>(executor);

            while (m_keepRunning.load())
            {
                auto cmd = GetCommandFromQueue();
                if (cmd == std::nullopt)
                {
                    expTimer->expires_after(1000ms);
                    co_await expTimer->async_wait(boost::asio::use_awaitable);
                    continue;
                }

                m_commandStore.StoreCommand(cmd.value());
                PopCommandFromQueue();

                co_spawn(
                    executor,
                    [cmd, DispatchCommand, this]() mutable -> boost::asio::awaitable<void>
                    {
                        cmd.value().ExecutionResult = co_await DispatchCommand(cmd.value());
                        m_commandStore.UpdateCommand(cmd.value());
                        LogInfo("Done processing command: {}({})", cmd.value().Command, cmd.value().Module);
                        co_return;
                    },
                    boost::asio::detached);
            }
        }

        void Stop();

    private:
        std::atomic<bool> m_keepRunning = true;
        command_store::CommandStore m_commandStore;
    };
} // namespace command_handler
