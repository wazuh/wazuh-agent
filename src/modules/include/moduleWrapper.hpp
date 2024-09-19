#pragma once

#include <functional>
#include <string>
#include <configuration_parser.hpp>

using namespace std;

struct ModuleWrapper {
    function<void()> Start;
    function<void(const configuration::ConfigurationParser&)> Setup;
    function<void()> Stop;
    function<string(const string&)> Command;
    function<string()> Name;
};
