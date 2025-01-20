#pragma once

#include <moduleWrapper.hpp>
#include <multitype_queue.hpp>
#include <task_manager.hpp>

#include <boost/asio/awaitable.hpp>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

class ModuleManager
{
public:
    ModuleManager(const std::function<int(Message)>& pushMessage,
                  std::shared_ptr<configuration::ConfigurationParser> configurationParser,
                  std::string uuid)
        : m_pushMessage(pushMessage)
        , m_configurationParser(std::move(configurationParser))
        , m_agentUUID(std::move(uuid))
    {
    }

    ~ModuleManager() = default;

    template<typename T>
    void AddModule(T& module)
    {
        const std::string& moduleName = module.Name();
        if (m_modules.find(moduleName) != m_modules.end())
        {
            throw std::runtime_error("Module '" + moduleName + "' already exists.");
        }

        module.SetPushMessageFunction(m_pushMessage);

        auto wrapper = std::make_shared<ModuleWrapper>(ModuleWrapper {
            .Start = [&module]() { module.Start(); },
            .Setup = [&module](std::shared_ptr<const configuration::ConfigurationParser> configurationParser)
            { module.Setup(configurationParser); },
            .Stop = [&module]() { module.Stop(); },
            .ExecuteCommand = [&module](std::string command, nlohmann::json parameters) -> Co_CommandExecutionResult
            { co_return co_await module.ExecuteCommand(command, parameters); },
            .Name =
                [&module]()
            {
                return module.Name();
            }});

        m_modules[moduleName] = wrapper;
    }

    void AddModules();
    std::shared_ptr<ModuleWrapper> GetModule(const std::string& name);

    /// @brief Start the modules
    ///
    /// This function begins the procedure to start the modules and blocks until the Start function
    /// for each module has been called. However, it does not guarantee that the modules are fully
    /// operational upon return; they may still be in the process of initializing.
    ///
    /// @note Call this function before interacting with the modules to ensure the startup process is initiated.
    /// @warning Ensure the modules have fully started before performing any operations that depend on them.
    void Start();
    void Setup();
    void Stop();

private:
    TaskManager m_taskManager;
    std::map<std::string, std::shared_ptr<ModuleWrapper>> m_modules;
    std::function<int(Message)> m_pushMessage;
    std::shared_ptr<configuration::ConfigurationParser> m_configurationParser;
    std::string m_agentUUID;
    std::mutex m_mutex;
    std::atomic<int> m_started {0};
};
