#pragma once

#include <functional>
#include <string>
#include <configuration_parser.hpp>


struct ModuleWrapper {
    std::function<void()> Start;
    std::function<void(const configuration::ConfigurationParser&)> Setup;
    std::function<void()> Stop;
    std::function<std::string(const std::string&)> Command;
    std::function<std::string()> Name;
};
