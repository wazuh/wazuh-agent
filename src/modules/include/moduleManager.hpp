#pragma once

#include <map>
#include <memory>
#include <string>
#include <thread>
#include <multitype_queue.hpp>
#include <moduleWrapper.hpp>

using namespace std;

template<typename T>
concept Module = requires(T t, const configuration::ConfigurationParser& configurationParser, const string & query,
                             const std::shared_ptr<IMultiTypeQueue> queue) {
    { t.Start() } -> same_as<void>;
    { t.Setup(configurationParser) } -> same_as<void>;
    { t.Stop() } -> same_as<void>;
    { t.Command(query) } -> same_as<string>;
    { t.Name() } -> same_as<string>;
    { t.SetMessageQueue(queue) } -> same_as<void>;
};

class ModuleManager {
public:
    ModuleManager(const std::shared_ptr<MultiTypeQueue>& messageQueue,
                const configuration::ConfigurationParser& configurationParser)
        : m_multiTypeQueue(messageQueue)
        , m_configurationParser(configurationParser)
    {}
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
            .Setup = [&module](const configuration::ConfigurationParser& configurationParser) { module.Setup(configurationParser); },
            .Stop = [&module]() { module.Stop(); },
            .Command = [&module](const string & query) { return module.Command(query); },
            .Name = [&module]() { return module.Name(); }
        });

        m_modules[moduleName] = wrapper;
    }

    shared_ptr<ModuleWrapper> GetModule(const string & name);
    void Start();
    void Setup();
    void Stop();

private:
    map<string, shared_ptr<ModuleWrapper>> m_modules;
    vector<thread> m_threads;
    std::shared_ptr<IMultiTypeQueue> m_multiTypeQueue;
    configuration::ConfigurationParser m_configurationParser;
};
