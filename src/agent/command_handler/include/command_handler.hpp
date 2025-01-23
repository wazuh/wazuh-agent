#pragma once

#include <command_entry.hpp>
#include <configuration_parser.hpp>

#include <config.h>
#include <logger.hpp>

#include <boost/asio.hpp>

#include <atomic>
#include <chrono>
#include <memory>
#include <optional>
#include <string>
#include <vector>

namespace command_store
{
    class CommandStore;
} // namespace command_store

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
        /// @param configurationParser Pointer to the configuration parser
        CommandHandler(std::shared_ptr<configuration::ConfigurationParser> configurationParser);

        /// @brief CommandHandler destructor
        ~CommandHandler();

        /// @brief Processes commands asynchronously
        ///
        /// This task retrieves commands from the queue and dispatches them for execution.
        /// If no command is available, it waits for a specified duration before retrying.
        ///
        /// @param getCommandFromQueue Function to retrieve a command from the queue
        /// @param popCommandFromQueue Function to remove a command from the queue
        /// @param reportCommandResult Function to report a command result
        /// @param dispatchCommand Function to dispatch the command for execution
        boost::asio::awaitable<void>
        CommandsProcessingTask(const std::function<std::optional<module_command::CommandEntry>()> getCommandFromQueue,
                               const std::function<void()> popCommandFromQueue,
                               const std::function<void(module_command::CommandEntry&)> reportCommandResult,
                               const std::function<boost::asio::awaitable<module_command::CommandExecutionResult>(
                                   module_command::CommandEntry&)> dispatchCommand);

        /// @brief Stops the command handler
        void Stop();

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
        std::unique_ptr<command_store::CommandStore> m_commandStore;
    };
} // namespace command_handler
