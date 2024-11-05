#pragma once

#include <imultitype_queue.hpp>
#include <moduleWrapper.hpp>
#include <module_command/command_entry.hpp>

#include <boost/asio/awaitable.hpp>
#include <nlohmann/json.hpp>

#include <functional>
#include <memory>
#include <string>

boost::asio::awaitable<module_command::CommandExecutionResult>
DispatchCommand(module_command::CommandEntry commandEntry,
                std::function<boost::asio::awaitable<module_command::CommandExecutionResult>(
                    std::string command, nlohmann::json parameters)> executeFunction,
                std::shared_ptr<IMultiTypeQueue> messageQueue);

boost::asio::awaitable<module_command::CommandExecutionResult>
DispatchCommand(module_command::CommandEntry commandEntry,
                std::shared_ptr<ModuleWrapper> module,
                std::shared_ptr<IMultiTypeQueue> messageQueue);
