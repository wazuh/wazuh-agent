#pragma once

#include <command_entry.hpp>
#include <imultitype_queue.hpp>
#include <moduleWrapper.hpp>

#include <boost/asio/awaitable.hpp>
#include <nlohmann/json.hpp>

#include <functional>
#include <memory>
#include <string>

/// @brief Dispatch a command to be executed locally
///
/// This function takes a command entry and a function to execute the command.
///
/// @param commandEntry The command entry to dispatch
/// @param executeFunction The function that will execute the command
/// @param messageQueue The message queue to send the result to
/// @return The result of the command execution
boost::asio::awaitable<module_command::CommandExecutionResult>
DispatchCommand(module_command::CommandEntry commandEntry,
                std::function<boost::asio::awaitable<module_command::CommandExecutionResult>(
                    std::string command, nlohmann::json parameters)> executeFunction,
                std::shared_ptr<IMultiTypeQueue> messageQueue);

/// @brief Dispatch a command to the proper module
///
/// This function takes a command entry and a module and dispatches
/// the command to be executed by the module.
///
/// @param commandEntry The command entry to dispatch
/// @param module The module that will execute the command
/// @param messageQueue The message queue to send the result to
/// @return The result of the command execution
boost::asio::awaitable<module_command::CommandExecutionResult>
DispatchCommand(module_command::CommandEntry commandEntry,
                std::shared_ptr<ModuleWrapper> module,
                std::shared_ptr<IMultiTypeQueue> messageQueue);

/// @brief Reports the result of a command execution to the message queue
///
/// This function takes a command entry and a message queue and reports
/// the result of the command execution.
///
/// @param commandEntry The command entry to report
/// @param messageQueue The message queue to send the result to
void ReportCommandResult(const module_command::CommandEntry& commandEntry,
                         std::shared_ptr<IMultiTypeQueue> messageQueue);
