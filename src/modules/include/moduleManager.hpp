#pragma once

#include <map>
#include <memory>
#include <string>
#include <thread>
#include <multitype_queue.hpp>
#include <moduleWrapper.hpp>


template<typename T>
concept Module = requires(T t, const configuration::ConfigurationParser& configurationParser, const std::string & query,
                             const std::shared_ptr<IMultiTypeQueue> queue) {
    { t.Start() } -> std::same_as<void>;
    { t.Setup(configurationParser) } -> std::same_as<void>;
    { t.Stop() } -> std::same_as<void>;
    { t.Command(query) } -> std::same_as<std::string>;
    { t.Name() } -> std::same_as<std::string>;
    { t.SetMessageQueue(queue) } -> std::same_as<void>;
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

        auto wrapper = std::make_shared<ModuleWrapper>(ModuleWrapper{
            .Start = [&module]() { module.Start(); },
            .Setup = [&module](const configuration::ConfigurationParser& configurationParser) { module.Setup(configurationParser); },
            .Stop = [&module]() { module.Stop(); },
            .Command = [&module](const std::string & query) { return module.Command(query); },
            .Name = [&module]() { return module.Name(); }
        });

        m_modules[moduleName] = wrapper;
    }

    std::shared_ptr<ModuleWrapper> GetModule(const std::string & name);
    void Start();
    void Setup();
    void Stop();

private:
    std::map<std::string, std::shared_ptr<ModuleWrapper>> m_modules;
    std::vector<std::thread> m_threads;
    std::shared_ptr<IMultiTypeQueue> m_multiTypeQueue;
    configuration::ConfigurationParser m_configurationParser;
};
