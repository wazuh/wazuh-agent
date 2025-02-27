#pragma once

#include <command_entry.hpp>

#include <boost/asio/awaitable.hpp>

#include <functional>
#include <optional>

namespace command_handler
{
    /// @brief Interface for the CommandHandler class
    class ICommandHandler
    {
    public:
        virtual ~ICommandHandler() = default;

        /// @brief Processes commands asynchronously
        ///
        /// This task retrieves commands from the queue and dispatches them for execution.
        /// If no command is available, it waits for a specified duration before retrying.
        ///
        /// @param getCommandFromQueue Function to retrieve a command from the queue
        /// @param popCommandFromQueue Function to remove a command from the queue
        /// @param reportCommandResult Function to report a command result
        /// @param dispatchCommand Function to dispatch the command for execution
        virtual boost::asio::awaitable<void>
        CommandsProcessingTask(const std::function<std::optional<module_command::CommandEntry>()> getCommandFromQueue,
                               const std::function<void()> popCommandFromQueue,
                               const std::function<void(module_command::CommandEntry&)> reportCommandResult,
                               const std::function<boost::asio::awaitable<module_command::CommandExecutionResult>(
                                   module_command::CommandEntry&)> dispatchCommand) = 0;

        /// @brief Stops the command handler
        virtual void Stop() = 0;
    };
} // namespace command_handler
