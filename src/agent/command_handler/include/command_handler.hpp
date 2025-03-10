#pragma once

#include <command_entry.hpp>
#include <configuration_parser.hpp>
#include <icommand_handler.hpp>
#include <icommand_store.hpp>

#include <boost/asio/awaitable.hpp>

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace command_handler
{
    /// @brief CommandHandler class
    ///
    /// This class is responsible for executing commands retrieved from the command
    /// store. It provides a way to dispatch commands to the corresponding
    /// command handlers and manage the command execution results.
    class CommandHandler : public ICommandHandler
    {
    public:
        /// @brief CommandHandler constructor
        /// @param configurationParser Pointer to the configuration parser
        /// @param commandStore Unique pointer to the command store
        CommandHandler(std::shared_ptr<configuration::ConfigurationParser> configurationParser,
                       std::unique_ptr<command_store::ICommandStore> commandStore = nullptr);

        /// @brief CommandHandler destructor
        ~CommandHandler();

        /// @copydoc ICommandHandler::CommandsProcessingTask
        boost::asio::awaitable<void>
        CommandsProcessingTask(const std::function<std::optional<module_command::CommandEntry>()> getCommandFromQueue,
                               const std::function<void()> popCommandFromQueue,
                               const std::function<void(module_command::CommandEntry&)> reportCommandResult,
                               const std::function<boost::asio::awaitable<module_command::CommandExecutionResult>(
                                   module_command::CommandEntry&)> dispatchCommand) override;

        /// @copydoc ICommandHandler::Stop
        void Stop() override;

    private:
        /// @brief Clean up commands that are in progress when the agent is stopped
        ///
        /// This function will set the status of all commands that are currently in
        /// progress to FAILED and update the command store. It will also call the
        /// reportCommandResult function for each command to report the result.
        ///
        /// @param reportCommandResult The function to report the command result
        void CleanUpInProgressCommands(std::function<void(module_command::CommandEntry&)> reportCommandResult);

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

        /// @brief Unique pointer to the command store
        std::unique_ptr<command_store::ICommandStore> m_commandStore;
    };
} // namespace command_handler
