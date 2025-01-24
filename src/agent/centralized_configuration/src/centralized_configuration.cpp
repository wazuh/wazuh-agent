#include <centralized_configuration.hpp>
#include <config.h>
#include <logger.hpp>

#include <filesystem>

namespace centralized_configuration
{
    CentralizedConfiguration::CentralizedConfiguration(SetGroupIdFunctionType setGroupIdFunction,
                                                       GetGroupIdFunctionType getGroupIdFunction,
                                                       DownloadGroupFilesFunctionType downloadGroupFilesFunction,
                                                       ValidateFileFunctionType validateFileFunction,
                                                       ReloadModulesFunctionType reloadModulesFunction,
                                                       std::shared_ptr<IFileSystem> fileSystemWrapper)
        : m_setGroupIdFunction(std::move(setGroupIdFunction))
        , m_getGroupIdFunction(std::move(getGroupIdFunction))
        , m_downloadGroupFilesFunction(std::move(downloadGroupFilesFunction))
        , m_validateFileFunction(std::move(validateFileFunction))
        , m_reloadModulesFunction(std::move(reloadModulesFunction))
        , m_fileSystemWrapper(fileSystemWrapper ? fileSystemWrapper
                                                : std::make_shared<filesystem_wrapper::FileSystemWrapper>())
    {
        if (m_setGroupIdFunction == nullptr || m_getGroupIdFunction == nullptr ||
            m_downloadGroupFilesFunction == nullptr || m_validateFileFunction == nullptr ||
            m_reloadModulesFunction == nullptr)
        {
            throw std::runtime_error("SetGroupIdFunction, GetGroupIdFunction, DownloadGroupFilesFunction, "
                                     "ValidateFileFunction, and ReloadModulesFunction must be provided.");
        }
    }

    boost::asio::awaitable<module_command::CommandExecutionResult> CentralizedConfiguration::ExecuteCommand(
        const std::string command,       // NOLINT(performance-unnecessary-value-param)
        const nlohmann::json parameters) // NOLINT(performance-unnecessary-value-param)
    {
        try
        {
            std::vector<std::string> groupIds {};

            if (command == module_command::SET_GROUP_COMMAND)
            {
                groupIds = parameters.at(module_command::GROUPS_ARG).get<std::vector<std::string>>();
                if (!std::all_of(groupIds.begin(), groupIds.end(), [](const std::string& id) { return !id.empty(); }))
                {
                    LogWarn("Group name can not be an empty string.");
                    co_return module_command::CommandExecutionResult {
                        module_command::Status::FAILURE,
                        "CentralizedConfiguration group set failed, a group name can not be an empty string."};
                }

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

                    if (m_fileSystemWrapper->exists(sharedDirPath) && m_fileSystemWrapper->is_directory(sharedDirPath))
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
            else if (command == module_command::FETCH_CONFIG_COMMAND)
            {
                groupIds = m_getGroupIdFunction();
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
} // namespace centralized_configuration
