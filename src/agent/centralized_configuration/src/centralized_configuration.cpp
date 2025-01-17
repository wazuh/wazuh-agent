#include <centralized_configuration.hpp>
#include <config.h>
#include <logger.hpp>

#include <filesystem>

namespace centralized_configuration
{
    boost::asio::awaitable<module_command::CommandExecutionResult> CentralizedConfiguration::ExecuteCommand(
        const std::string command,       // NOLINT(performance-unnecessary-value-param)
        const nlohmann::json parameters) // NOLINT(performance-unnecessary-value-param)
    {
        try
        {
            std::vector<std::string> groupIds {};

            if (command == "set-group")
            {
                if (m_setGroupIdFunction && m_downloadGroupFilesFunction && m_validateFileFunction &&
                    m_reloadModulesFunction)
                {
                    groupIds = parameters.get<std::vector<std::string>>();

                    if (!m_setGroupIdFunction(groupIds))
                    {
                        LogWarn("Group set failed, error saving group information");
                        co_return module_command::CommandExecutionResult {
                            module_command::Status::FAILURE,
                            "CentralizedConfiguration group set failed, error saving group information"};
                    }

                    try
                    {
                        std::filesystem::path sharedDirPath(config::DEFAULT_SHARED_CONFIG_PATH);

                        if (m_fileSystemWrapper->exists(sharedDirPath) &&
                            m_fileSystemWrapper->is_directory(sharedDirPath))
                        {
                            for (const auto& entry : std::filesystem::directory_iterator(sharedDirPath))
                            {
                                m_fileSystemWrapper->remove_all(entry);
                            }
                        }
                    }
                    catch (const std::filesystem::filesystem_error& e)
                    {
                        LogWarn("Error while cleaning the shared directory {}.", e.what());
                        co_return module_command::CommandExecutionResult {
                            module_command::Status::FAILURE,
                            "CentralizedConfiguration group set failed, error while cleaning the shared directory"};
                    }
                }
                else
                {
                    LogWarn("Group set failed, one of the required functions has not been set.");
                    co_return module_command::CommandExecutionResult {
                        module_command::Status::FAILURE,
                        "CentralizedConfiguration group set failed, one of the required functions has not been set."};
                }
            }
            else if (command == "update-group")
            {
                if (m_getGroupIdFunction && m_downloadGroupFilesFunction && m_validateFileFunction &&
                    m_reloadModulesFunction)
                {
                    groupIds = m_getGroupIdFunction();
                }
                else
                {
                    LogWarn("Group update failed, one of the required functions has not been set.");
                    co_return module_command::CommandExecutionResult {
                        module_command::Status::FAILURE,
                        "CentralizedConfiguration group update failed, one of the required functions has not been "
                        "set."};
                }
            }
            else
            {
                LogWarn("CentralizedConfiguration command not recognized");
                co_return module_command::CommandExecutionResult {module_command::Status::FAILURE,
                                                                  "CentralizedConfiguration command not recognized"};
            }

            for (const auto& groupId : groupIds)
            {
                const std::filesystem::path tmpGroupFile =
                    m_fileSystemWrapper->temp_directory_path() / (groupId + config::DEFAULT_SHARED_FILE_EXTENSION);

                const auto dlResult = co_await m_downloadGroupFilesFunction(groupId, tmpGroupFile.string());

                if (!dlResult)
                {
                    LogWarn("Failed to download the file for group '{}'", groupId);
                    co_return module_command::CommandExecutionResult {
                        module_command::Status::FAILURE,
                        "CentralizedConfiguration failed to download the file for group '" + groupId + "'"};
                }

                if (!m_validateFileFunction(tmpGroupFile))
                {
                    LogWarn("Failed to validate the file for group '{}', invalid group file received: {}",
                            groupId,
                            tmpGroupFile.string());

                    try
                    {
                        if (m_fileSystemWrapper->exists(tmpGroupFile) &&
                            tmpGroupFile.parent_path() == m_fileSystemWrapper->temp_directory_path())
                        {
                            if (!m_fileSystemWrapper->remove(tmpGroupFile))
                            {
                                LogWarn("Failed to delete invalid group file: {}", tmpGroupFile.string());
                            }
                        }
                    }
                    catch (const std::filesystem::filesystem_error& e)
                    {
                        LogWarn("Error while trying to delete invalid group file: {}. Exception: {}",
                                tmpGroupFile.string(),
                                e.what());
                    }

                    co_return module_command::CommandExecutionResult {
                        module_command::Status::FAILURE,
                        "CentralizedConfiguration validate file failed, invalid file received."};
                }

                const std::filesystem::path destGroupFile = std::filesystem::path(config::DEFAULT_SHARED_CONFIG_PATH) /
                                                            (groupId + config::DEFAULT_SHARED_FILE_EXTENSION);

                try
                {
                    m_fileSystemWrapper->create_directories(destGroupFile.parent_path());
                    m_fileSystemWrapper->rename(tmpGroupFile, destGroupFile);
                }
                catch (const std::filesystem::filesystem_error& e)
                {
                    LogWarn("Failed to move file to destination: {}. Error: {}", destGroupFile.string(), e.what());
                    co_return module_command::CommandExecutionResult {module_command::Status::FAILURE,
                                                                      "Failed to move shared file to destination."};
                }
            }

            m_reloadModulesFunction();

            const std::string messageOnSuccess = "CentralizedConfiguration " + command + " done.";
            co_return module_command::CommandExecutionResult {module_command::Status::SUCCESS, messageOnSuccess};
        }
        catch (const nlohmann::json::exception&)
        {
            LogWarn("CentralizedConfiguration error while parsing parameters");
            co_return module_command::CommandExecutionResult {
                module_command::Status::FAILURE, "CentralizedConfiguration error while parsing parameters"};
        }
    }

    void CentralizedConfiguration::SetGroupIdFunction(SetGroupIdFunctionType setGroupIdFunction)
    {
        m_setGroupIdFunction = std::move(setGroupIdFunction);
    }

    void CentralizedConfiguration::GetGroupIdFunction(GetGroupIdFunctionType getGroupIdFunction)
    {
        m_getGroupIdFunction = std::move(getGroupIdFunction);
    }

    void
    CentralizedConfiguration::SetDownloadGroupFilesFunction(DownloadGroupFilesFunctionType downloadGroupFilesFunction)
    {
        m_downloadGroupFilesFunction = std::move(downloadGroupFilesFunction);
    }

    void CentralizedConfiguration::ValidateFileFunction(ValidateFileFunctionType validateFileFunction)
    {
        m_validateFileFunction = std::move(validateFileFunction);
    }

    void CentralizedConfiguration::ReloadModulesFunction(ReloadModulesFunctionType reloadModulesFunction)
    {
        m_reloadModulesFunction = std::move(reloadModulesFunction);
    }
} // namespace centralized_configuration
