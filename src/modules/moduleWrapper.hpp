#pragma once

#include <functional>
#include <string>
#include "configuration.hpp"

using namespace std;

struct ModuleWrapper {
    function<void()> run;
    function<int(const Configuration&)> setup;
    function<void()> stop;
    function<string(const string&)> command;
    function<string()> name;
};
