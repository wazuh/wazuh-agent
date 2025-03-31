#pragma once

#include <command_entry.hpp>
#include <configuration_parser.hpp>

#include <boost/asio/awaitable.hpp>

#include <nlohmann/json.hpp>

#include <functional>
#include <string>

using Co_CommandExecutionResult = boost::asio::awaitable<module_command::CommandExecutionResult>;

struct ModuleWrapper
{
    std::function<void()> Start;
    std::function<void(std::shared_ptr<const configuration::ConfigurationParser>)> Setup;
    std::function<void()> Stop;
    std::function<Co_CommandExecutionResult(std::string, nlohmann::json)> ExecuteCommand;
    std::function<std::string()> Name;
};
