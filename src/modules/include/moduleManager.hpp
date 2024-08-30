#pragma once

#include <map>
#include <memory>
#include <string>
#include <thread>
#include <configuration.hpp>
#include <multitype_queue.hpp>
#include <moduleWrapper.hpp>

using namespace std;

template<typename T>
concept Module = requires(T t, const Configuration & config, const string & query,
                             const std::shared_ptr<IMultiTypeQueue> queue) {
    { t.start() } -> same_as<void>;
    { t.setup(config) } -> same_as<int>;
    { t.stop() } -> same_as<void>;
    { t.command(query) } -> same_as<string>;
    { t.name() } -> same_as<string>;
    { t.setMessageQueue(queue) } -> same_as<void>;
};

class ModuleManager {
public:
    ModuleManager() = default;
    ~ModuleManager() = default;

    template <typename T>
    void addModule(T& module) {
        const std::string& moduleName = module.name();
        if (modules.find(moduleName) != modules.end()) {
            throw std::runtime_error("Module '" + moduleName + "' already exists.");
        }

        module.setMessageQueue(multiTypeQueue);

        auto wrapper = make_shared<ModuleWrapper>(ModuleWrapper{
            .start = [&module]() { module.start(); },
            .setup = [&module](const Configuration & config) { return module.setup(config); },
            .stop = [&module]() { module.stop(); },
            .command = [&module](const string & query) { return module.command(query); },
            .name = [&module]() { return module.name(); }
        });

        modules[moduleName] = wrapper;
    }

    shared_ptr<ModuleWrapper> getModule(const string & name);
    void start();
    void setup(const Configuration & config);
    void stop();
    void setMessageQueue(const std::shared_ptr<MultiTypeQueue>& messageQueue) {
        multiTypeQueue = messageQueue;
    }

private:
    map<string, shared_ptr<ModuleWrapper>> modules;
    vector<thread> threads;
    std::shared_ptr<IMultiTypeQueue> multiTypeQueue;
};
