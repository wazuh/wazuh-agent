#pragma once

#include <module_command/command_entry.hpp>

#include <moduleWrapper.hpp>
#include <multitype_queue.hpp>

#include <boost/asio/awaitable.hpp>

#include <functional>
#include <memory>
#include <tuple>
#include <vector>

boost::asio::awaitable<module_command::CommandExecutionResult>
DispatchCommand(module_command::CommandEntry commandEntry,
                std::function<boost::asio::awaitable<module_command::CommandExecutionResult>(
                    std::string command, std::vector<std::string> parameters)> executeFunction,
                std::shared_ptr<IMultiTypeQueue> messageQueue);

boost::asio::awaitable<module_command::CommandExecutionResult>
DispatchCommand(module_command::CommandEntry commandEntry,
                std::shared_ptr<ModuleWrapper> module,
                std::shared_ptr<IMultiTypeQueue> messageQueue);
