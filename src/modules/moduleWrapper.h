#ifndef MODULE_WRAPPER_H
#define MODULE_WRAPPER_H

#include <functional>
#include <string>
#include "configuration.h"

using namespace std;

struct ModuleWrapper {
    function<void()> run;
    function<int(const Configuration&)> setup;
    function<void()> stop;
    function<string(const string&)> command;
    function<string()> name;
};

#endif // MODULE_WRAPPER_H
