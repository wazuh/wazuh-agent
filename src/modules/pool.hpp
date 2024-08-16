#pragma once

#include <map>
#include <memory>
#include <string>
#include <thread>
#include "configuration.hpp"
#include "moduleWrapper.hpp"

using namespace std;

template<typename T>
concept Module = requires(T t, const Configuration & config, const string & query) {
    { t.start() } -> same_as<void *>;
    { t.setup(config) } -> same_as<int>;
    { t.stop() } -> same_as<void>;
    { t.command(query) } -> same_as<string>;
    { t.name() } -> same_as<string>;
};

class Pool {
public:
    Pool();

    template <typename T>
    void addModule(T& module);

    shared_ptr<ModuleWrapper> getModule(const string & name);
    void start();
    void setup(const Configuration & config);
    void stop();

private:
    map<string, shared_ptr<ModuleWrapper>> modules;
    vector<thread> threads;
};
