#ifndef MODULE_WRAPPER_H
#define MODULE_WRAPPER_H

#include <functional>
#include <string>
#include "configuration.h"

using namespace std;

struct ModuleWrapper {
    function<void()> run;                              // Main function
    function<int(const Configuration&)> setup;
    function<void(void *)> destroy;                     // Configuration destructor
    function<void()> stop;                              // Module destructor
    function<string(const string&)> command;
    function<size_t(void *, char *, char **)> query;    // Run a query
    function<string()> name;
    function<string(int(const string&))> sync;          // Sync
    function<string(const void *)>dump;                 // Dump current configuration
};

#endif // MODULE_WRAPPER_H
