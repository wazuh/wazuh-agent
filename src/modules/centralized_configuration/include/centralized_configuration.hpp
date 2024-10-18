#pragma once

#include <configuration_parser.hpp>
#include <moduleWrapper.hpp>
#include <multitype_queue.hpp>

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace centralized_configuration
{
    class CentralizedConfiguration
    {
    public:
        using SetGroupIdFunctionType = std::function<void(const std::vector<std::string>& groupList)>;
        using GetGroupIdFunctionType = std::function<std::vector<std::string>()>;
        using DownloadGroupFilesFunctionType = std::function<bool(const std::string& group, const std::string& dstFilePath)>;

        /**
         * @brief Starts the centralized configuration process.
         * @details This is a no-op function, required by the ModuleWrapper interface
         * to be used within the ModuleManager.
         */
        void Start() const;

        /**
         * @brief Configures the centralized configuration system.
         * @param configParser The configuration parser containing settings to be applied.
         * @details This is a no-op function, required by the ModuleWrapper interface
         * to be used within the ModuleManager.
         */
        void Setup(const configuration::ConfigurationParser&) const;

        /**
         * @brief Stops the centralized configuration process.
         * @details This is a no-op function, required by the ModuleWrapper interface
         * to be used within the ModuleManager.
         */
        void Stop() const;

        /**
         * @brief Executes a command for the centralized configuration system.
         * @param command A string containing a JSON command to execute.
         * @details Required by the ModuleWrapper interface to be used within the ModuleManager
         * @return Co_CommandExecutionResult The result of executing the command, either success or failure.
         */
        Co_CommandExecutionResult ExecuteCommand(std::string command);

        /**
         * @brief Gets the name of the centralized configuration system.
         * @details Required by the ModuleWrapper interface to be used within the ModuleManager
         * @return A string representing the name.
         */
        std::string Name() const;

        /**
         * @brief Sets the message queue for the system.
         * @param queue A shared pointer to the message queue used for communication.
         * @details This is a no-op function, required by the ModuleWrapper interface
         * to be used within the ModuleManager.
         */
        void SetMessageQueue(const std::shared_ptr<IMultiTypeQueue> queue);

        /**
         * @brief Sets the function to assign group IDs.
         * @details The "set-group" commands requires such function to be set.
         * @param setGroupIdFunction A function to set group IDs.
         */
        void SetGroupIdFunction(SetGroupIdFunctionType setGroupIdFunction);

        /**
         * @brief Sets the function to retrieve group IDs.
         * @details The "update-group" commands requires such function to be set.
         * @param getGroupIdFunction A function to get group IDs.
         */
        void GetGroupIdFunction(GetGroupIdFunctionType getGroupIdFunction);

        /**
         * @brief Sets the function to download group configuration files.
         * @details Configures how and where to download group configuration files.
         * These will be used to set and update the Agent groups via the "set-group" and "update-group"
         * commands.
         * @param downloadGroupFilesFunction A function to download files for a given group ID.
         */
        void SetDownloadGroupFilesFunction(DownloadGroupFilesFunctionType downloadGroupFilesFunction);

    private:
        SetGroupIdFunctionType m_setGroupIdFunction;
        GetGroupIdFunctionType m_getGroupIdFunction;
        DownloadGroupFilesFunctionType m_downloadGroupFilesFunction;
    };
} // namespace centralized_configuration
