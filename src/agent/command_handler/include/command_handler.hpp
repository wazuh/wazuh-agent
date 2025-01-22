#pragma once

#include <command_store.hpp>
#include <logger.hpp>

#include <boost/asio.hpp>

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <string>

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
        /// @param ReportCommandResult Function to report a command result
        /// @param DispatchCommand Function to dispatch the command for execution
        template<typename T>
        boost::asio::awaitable<void> CommandsProcessingTask(
            const std::function<std::optional<T>()> GetCommandFromQueue,
            const std::function<void()> PopCommandFromQueue,
            const std::function<void(T&)> ReportCommandResult,
            const std::function<boost::asio::awaitable<module_command::CommandExecutionResult>(T&)> DispatchCommand)
        {
            using namespace std::chrono_literals;
            const auto executor = co_await boost::asio::this_coro::executor;
            std::unique_ptr<boost::asio::steady_timer> expTimer = std::make_unique<boost::asio::steady_timer>(executor);

            CleanUpInProgressCommands(ReportCommandResult);

            while (m_keepRunning.load())
            {
                auto cmd = GetCommandFromQueue();
                if (cmd == std::nullopt)
                {
                    expTimer->expires_after(1000ms);
                    co_await expTimer->async_wait(boost::asio::use_awaitable);
                    continue;
                }

                LogDebug("Processing command: {}({})", cmd.value().Command, cmd.value().Parameters.dump());

                if (!CheckCommand(cmd.value()))
                {
                    cmd.value().ExecutionResult.ErrorCode = module_command::Status::FAILURE;
                    cmd.value().ExecutionResult.Message = "Command is not valid";
                    LogError("Error checking module and args for command: {} {}. Error: {}",
                             cmd.value().Id,
                             cmd.value().Command,
                             cmd.value().ExecutionResult.Message);
                    ReportCommandResult(cmd.value());
                    PopCommandFromQueue();
                    continue;
                }

                if (!m_commandStore.StoreCommand(cmd.value()))
                {
                    cmd.value().ExecutionResult.ErrorCode = module_command::Status::FAILURE;
                    cmd.value().ExecutionResult.Message = "Agent's database failure";
                    LogError("Error storing command: {} {}. Error: {}",
                             cmd.value().Id,
                             cmd.value().Command,
                             cmd.value().ExecutionResult.Message);
                    ReportCommandResult(cmd.value());
                    PopCommandFromQueue();
                    continue;
                }

                PopCommandFromQueue();

                if (cmd.value().ExecutionMode == module_command::CommandExecutionMode::SYNC)
                {
                    cmd.value().ExecutionResult = co_await DispatchCommand(cmd.value());
                    m_commandStore.UpdateCommand(cmd.value());
                    LogInfo("Done processing command: {}({})", cmd.value().Command, cmd.value().Module);
                }
                else
                {
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
        }

        /// @brief Stops the command handler
        void Stop();

    private:
        /// @brief Clean up commands that are in progress when the agent is stopped
        ///
        /// This function will set the status of all commands that are currently in
        /// progress to FAILED and update the command store. It will also call the
        /// ReportCommandResult function for each command to report the result.
        ///
        /// @tparam T The type of the command
        /// @param ReportCommandResult The function to report the command result
        template<typename T>
        void CleanUpInProgressCommands(std::function<void(T&)> ReportCommandResult)
        {
            auto cmds = m_commandStore.GetCommandByStatus(module_command::Status::IN_PROGRESS);

            if (cmds != std::nullopt)
            {
                for (auto& cmd : *cmds)
                {
                    cmd.ExecutionResult.ErrorCode = module_command::Status::FAILURE;
                    cmd.ExecutionResult.Message = "Agent stopped during execution";
                    ReportCommandResult(cmd);
                    m_commandStore.UpdateCommand(cmd);
                }
            }
        }

        /// @brief Check if the command is valid
        ///
        /// This function checks if the given command is valid by looking it up in
        /// the map of valid commands. If the command is valid, it checks if the
        /// parameters are valid. If the command is not valid, it logs an error and
        /// returns false. If the command is valid, it sets the module for the
        /// command and returns true.
        ///
        /// @param cmd The command to check
        /// @return True if the command is valid, false otherwise
        bool CheckCommand(module_command::CommandEntry& cmd);

        /// @brief Indicates whether the command handler is running or not
        std::atomic<bool> m_keepRunning = true;

        /// @brief An instance of the command store
        command_store::CommandStore m_commandStore;
    };
} // namespace command_handler
