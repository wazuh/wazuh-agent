#pragma once

#include <command_entry.hpp>
#include <message.hpp>
#include <moduleWrapper.hpp>
// #include <sca.hpp>
#include <task_manager.hpp>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

class ModuleManager
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
    /// The module is wrapped in a ModuleWrapper and added to the map of
    /// modules under its name.
    ///
    /// @tparam T The type of the module
    /// @param[in] module The module to add
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

    /// @brief Adds all modules to the manager
    void AddModules();

    /// @brief Get a module by name
    ///
    /// @param[in] name Name of the module
    /// @return Pointer to the module
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

    /// @brief Setup the modules
    void Setup();

    /// @brief Stop the modules
    void Stop();

private:
    /// @brief The task manager
    TaskManager m_taskManager;

    /// @brief The modules
    std::map<std::string, std::shared_ptr<ModuleWrapper>> m_modules;

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

    // std::unique_ptr<ISecurityConfigurationAssessment> m_sca;
};
