#pragma once

#include <functional>
#include <string>
#include "configuration.hpp"

using namespace std;

struct ModuleWrapper {
    function<void()> Start;
    function<int(const Configuration&)> Setup;
    function<void()> Stop;
    function<string(const string&)> Command;
    function<string()> Name;
};
