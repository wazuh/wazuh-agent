#ifndef POOL_H
#define POOL_H

#include <map>
#include <memory>
#include <string>
#include "configuration.h"
#include "moduleWrapper.h"
// #include "logCollector.h"
// #include "fim.h"
#include "inventory.h"
// #include "sca.h"

using namespace std;

template<typename T>
concept Module = requires(T t, const Configuration & config, const string & query) {
    { t.run() } -> same_as<void>;
    { t.setup(config) } -> same_as<int>;
    { t.stop() } -> same_as<void>;
    { t.command(query) } -> same_as<string>;
    { t.name() } -> same_as<string>;
};

class Pool {
public:
    Pool();

    template <Module T>
    void addModule(shared_ptr<T> module);

    shared_ptr<ModuleWrapper> getModule(const string & name);
    void start();
    void setup(const Configuration & config);
    void stop();

private:
    map<string, shared_ptr<ModuleWrapper>> modules;
};

#endif // POOL_H
