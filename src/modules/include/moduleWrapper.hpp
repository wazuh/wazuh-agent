#pragma once

#include <configuration_parser.hpp>
#include <module_command/command_entry.hpp>

#include <boost/asio/awaitable.hpp>

#include <functional>
#include <string>
#include <vector>

using Co_CommandExecutionResult = boost::asio::awaitable<module_command::CommandExecutionResult>;

struct ModuleWrapper {
    std::function<void()> Start;
    std::function<void(const configuration::ConfigurationParser&)> Setup;
    std::function<void()> Stop;
    std::function<Co_CommandExecutionResult(std::string, std::vector<std::string>)> ExecuteCommand;
    std::function<std::string()> Name;
};
