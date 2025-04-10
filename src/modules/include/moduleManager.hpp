#pragma once

#include <command_entry.hpp>
#include <imodule.hpp>
#include <imoduleManager.hpp>
#include <message.hpp>
#include <task_manager.hpp>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

class ModuleManager : public IModuleManager
{
public:
    /// @brief Constructor for ModuleManager
    ///
    /// @param[in] pushMessage Callback that will be used to send messages to the manager
    /// @param[in] configurationParser Configuration parser for the modules
    /// @param[in] uuid Unique identifier of the Agent
    ModuleManager(const std::function<int(Message)>& pushMessage,
                  std::shared_ptr<configuration::ConfigurationParser> configurationParser,
                  std::string uuid);

    /// @brief Default destructor
    ~ModuleManager() = default;

    /// @brief Adds a module to the manager
    ///
    /// The module is added only if it doesn't already exist. The module's
    /// SetPushMessageFunction is set to the manager's pushMessage callback.
    ///
    /// @param[in] module The module to add
    void AddModule(std::shared_ptr<IModule> module);

    /// @copydoc IModuleManager::AddModules
    void AddModules() override;

    /// @copydoc IModuleManager::GetModule
    std::shared_ptr<IModule> GetModule(const std::string& name) override;

    /// @copydoc IModuleManager::Start
    void Start() override;

    /// @copydoc IModuleManager::Setup
    void Setup() override;

    /// @copydoc IModuleManager::Stop
    void Stop() override;

private:
    /// @brief The task manager
    TaskManager m_taskManager;

    /// @brief The modules
    std::map<std::string, std::shared_ptr<IModule>> m_modules;

    /// @brief The pushMessage callback
    std::function<int(Message)> m_pushMessage;

    /// @brief The configuration parser
    std::shared_ptr<configuration::ConfigurationParser> m_configurationParser;

    /// @brief The UUID of the agent
    std::string m_agentUUID;

    /// @brief The mutex for the modules
    std::mutex m_mutex;

    /// @brief The number of modules that have started
    std::atomic<int> m_started {0};
};
