#pragma once

#include <map>
#include <memory>
#include <string>
#include <thread>
#include <multitype_queue.hpp>
#include <moduleWrapper.hpp>

#include <boost/asio/awaitable.hpp>

class ModuleManager {
public:
    ModuleManager(const std::function<int(Message)>& pushMessage,
                configuration::ConfigurationParser configurationParser,
                const std::function<void(std::function<void()>)>& createTask)
        : m_pushMessage(pushMessage)
        , m_configurationParser(std::move(configurationParser))
        , m_createTask(createTask)
    {}
    ~ModuleManager() = default;

    template <typename T>
    void AddModule(T& module) {
        const std::string& moduleName = module.Name();
        if (m_modules.find(moduleName) != m_modules.end()) {
            throw std::runtime_error("Module '" + moduleName + "' already exists.");
        }

        module.SetPushMessageFunction(m_pushMessage);

        auto wrapper = std::make_shared<ModuleWrapper>(ModuleWrapper{
            .Start = [&module]() { module.Start(); },
            .Setup = [&module](const configuration::ConfigurationParser& configurationParser) { module.Setup(configurationParser); },
            .Stop = [&module]() { module.Stop(); },
            .ExecuteCommand = [&module](std::string query, nlohmann::json parameters) -> Co_CommandExecutionResult
            {
                co_return co_await module.ExecuteCommand(query, parameters);
            },
            .Name = [&module]() { return module.Name(); }
        });

        m_modules[moduleName] = wrapper;
    }

    void AddModules();
    std::shared_ptr<ModuleWrapper> GetModule(const std::string & name);
    void Start();
    void Setup();
    void Stop();

private:
    std::map<std::string, std::shared_ptr<ModuleWrapper>> m_modules;
    std::function<int(Message)> m_pushMessage;
    configuration::ConfigurationParser m_configurationParser;
    std::function<void(std::function<void()>)> m_createTask;
};
