#pragma once

#include <command_entry.hpp>
#include <message.hpp>
#include <moduleWrapper.hpp>
#include <task_manager.hpp>

#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <thread>

class IModuleManager
{
public:
    /// @brief Default destructor
    virtual ~IModuleManager() = default;

    /// @brief Adds all modules to the manager
    virtual void AddModules() = 0;

    /// @brief Get a module by name
    ///
    /// @param[in] name Name of the module
    /// @return Pointer to the module
    virtual std::shared_ptr<ModuleWrapper> GetModule(const std::string& name) = 0;

    /// @brief Start the modules
    ///
    /// This function begins the procedure to start the modules and blocks until the Start function
    /// for each module has been called. However, it does not guarantee that the modules are fully
    /// operational upon return; they may still be in the process of initializing.
    ///
    /// @note Call this function before interacting with the modules to ensure the startup process is initiated.
    /// @warning Ensure the modules have fully started before performing any operations that depend on them.
    virtual void Start() = 0;

    /// @brief Setup the modules
    virtual void Setup() = 0;

    /// @brief Stop the modules
    virtual void Stop() = 0;
};
