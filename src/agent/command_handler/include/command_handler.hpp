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
    /// @brief CommandHandler class
    ///
    /// This class is responsible for executing commands retrieved from the command
    /// store. It provides a way to dispatch commands to the corresponding
    /// command handlers and manage the command execution results.
    class CommandHandler
    {
    public:
        /// @brief CommandHandler constructor
        /// @param dbFolderPath The path to the database folder
        CommandHandler(const std::string& dbFolderPath)
            : m_commandStore(dbFolderPath)
        {
        }

        /// @brief Processes commands asynchronously
        ///
        /// This task retrieves commands from the queue and dispatches them for execution.
        /// If no command is available, it waits for a specified duration before retrying.
        ///
        /// @tparam T The type of the command to process
        /// @param GetCommandFromQueue Function to retrieve a command from the queue
        /// @param PopCommandFromQueue Function to remove a command from the queue
        /// @param DispatchCommand Function to dispatch the command for execution
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

        /// @brief Stops the command handler
        void Stop();

    private:
        /// @brief Indicates whether the command handler is running or not
        std::atomic<bool> m_keepRunning = true;

        /// @brief An instance of the command store
        command_store::CommandStore m_commandStore;
    };
} // namespace command_handler
