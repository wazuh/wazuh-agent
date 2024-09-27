#pragma once

#include <configuration_parser.hpp>

#include <boost/asio/awaitable.hpp>

#include <functional>
#include <string>


struct ModuleWrapper {
    std::function<void()> Start;
    std::function<void(const configuration::ConfigurationParser&)> Setup;
    std::function<void()> Stop;
    std::function<boost::asio::awaitable<std::string>(std::string)> Command;
    std::function<std::string()> Name;
};
