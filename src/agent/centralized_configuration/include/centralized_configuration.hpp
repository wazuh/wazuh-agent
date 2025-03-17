#pragma once

#include <command_entry.hpp>
#include <ifilesystem_wrapper.hpp>

#include <boost/asio/awaitable.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <functional>
#include <string>
#include <vector>

namespace centralized_configuration
{
    /// @brief CentralizedConfiguration class.
    class CentralizedConfiguration
    {
    public:
        using SetGroupIdFunctionType = std::function<bool(const std::vector<std::string>& groupList)>;
        using GetGroupIdFunctionType = std::function<std::vector<std::string>()>;
        using DownloadGroupFilesFunctionType =
            std::function<boost::asio::awaitable<bool>(std::string group, std::string dstFilePath)>;
        using ValidateFileFunctionType = std::function<bool(const std::filesystem::path& configFile)>;
        using ReloadModulesFunctionType = std::function<void()>;

        /// @brief Constructor that allows injecting a file system wrapper.
        /// @param setGroupIdFunction A function to set group IDs.
        /// @param getGroupIdFunction A function to get group IDs.
        /// @param downloadGroupFilesFunction A function to download files for a given group ID.
        /// @param validateFileFunction A function to validate a file.
        /// @param reloadModulesFunction A function to reload modules.
        /// @param fileSystemWrapper An optional filesystem wrapper. If nullptr, it will use FileSystemWrapper.
        explicit CentralizedConfiguration(SetGroupIdFunctionType setGroupIdFunction,
                                          GetGroupIdFunctionType getGroupIdFunction,
                                          DownloadGroupFilesFunctionType downloadGroupFilesFunction,
                                          ValidateFileFunctionType validateFileFunction,
                                          ReloadModulesFunctionType reloadModulesFunction,
                                          std::shared_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr);

        /// @brief Executes a command for the centralized configuration system.
        /// @param command A string containing a JSON command to execute.
        /// @param parameters A json object containing the parameters of the command to be executed.
        /// @return An awaitable with the result of executing the command, either success or failure.
        boost::asio::awaitable<module_command::CommandExecutionResult> ExecuteCommand(std::string command,
                                                                                      nlohmann::json parameters);

    private:
        /// @brief Function to set group IDs.
        SetGroupIdFunctionType m_setGroupIdFunction;

        /// @brief Function to get group IDs.
        GetGroupIdFunctionType m_getGroupIdFunction;

        /// @brief Function to download group configuration files.
        DownloadGroupFilesFunctionType m_downloadGroupFilesFunction;

        /// @brief Function to validate group configuration files.
        ValidateFileFunctionType m_validateFileFunction;

        /// @brief Function to reload modules.
        ReloadModulesFunctionType m_reloadModulesFunction;

        /// @brief Member to interact with the file system.
        std::shared_ptr<IFileSystemWrapper> m_fileSystemWrapper;
    };
} // namespace centralized_configuration
