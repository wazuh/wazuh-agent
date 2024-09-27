#pragma once

#include <configuration_parser.hpp>

#include <boost/asio/awaitable.hpp>

#include <functional>
#include <string>

struct CommandExecutionResult {
    int ErrorCode = 0;
    std::string Message = "";
};

using Co_CommandExecutionResult = boost::asio::awaitable<CommandExecutionResult>;

struct ModuleWrapper {
    std::function<void()> Start;
    std::function<void(const configuration::ConfigurationParser&)> Setup;
    std::function<void()> Stop;
    std::function<Co_CommandExecutionResult(std::string)> Command;
    std::function<std::string()> Name;
};
