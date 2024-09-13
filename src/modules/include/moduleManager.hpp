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
    { t.Start() } -> same_as<void>;
    { t.Setup(config) } -> same_as<int>;
    { t.Stop() } -> same_as<void>;
    { t.Command(query) } -> same_as<string>;
    { t.Name() } -> same_as<string>;
    { t.SetMessageQueue(queue) } -> same_as<void>;
};

class ModuleManager {
public:
    ModuleManager() = default;
    ~ModuleManager() = default;

    template <typename T>
    void AddModule(T& module) {
        const std::string& moduleName = module.Name();
        if (m_modules.find(moduleName) != m_modules.end()) {
            throw std::runtime_error("Module '" + moduleName + "' already exists.");
        }

        module.SetMessageQueue(m_multiTypeQueue);

        auto wrapper = make_shared<ModuleWrapper>(ModuleWrapper{
            .Start = [&module]() { module.Start(); },
            .Setup = [&module](const Configuration & config) { return module.Setup(config); },
            .Stop = [&module]() { module.Stop(); },
            .Command = [&module](const string & query) { return module.Command(query); },
            .Name = [&module]() { return module.Name(); }
        });

        m_modules[moduleName] = wrapper;
    }

    shared_ptr<ModuleWrapper> GetModule(const string & name);
    void Start();
    void Setup(const Configuration & config);
    void Stop();
    void SetMessageQueue(const std::shared_ptr<MultiTypeQueue>& messageQueue) {
        m_multiTypeQueue = messageQueue;
    }

private:
    map<string, shared_ptr<ModuleWrapper>> m_modules;
    vector<thread> m_threads;
    std::shared_ptr<IMultiTypeQueue> m_multiTypeQueue;
};
