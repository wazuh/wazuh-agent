#include <centralized_configuration.hpp>
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
            std::string messageOnSuccess {};

            if (command == "set-group")
            {
                if (m_setGroupIdFunction && m_downloadGroupFilesFunction)
                {
                    if (parameters.empty())
                    {
                        LogWarn("Group set failed, no group list");
                        co_return module_command::CommandExecutionResult {
                            module_command::Status::FAILURE,
                            "CentralizedConfiguration group set failed, no group list"};
                    }

                    groupIds = parameters[0].get<std::vector<std::string>>();
                    messageOnSuccess = "CentralizedConfiguration group set";

                    m_setGroupIdFunction(groupIds);
                }
                else
                {
                    LogWarn("Group set failed, no function set");
                    co_return module_command::CommandExecutionResult {
                        module_command::Status::FAILURE, "CentralizedConfiguration group set failed, no function set"};
                }
            }
            else if (command == "update-group")
            {
                if (m_getGroupIdFunction && m_downloadGroupFilesFunction)
                {
                    groupIds = m_getGroupIdFunction();
                    messageOnSuccess = "CentralizedConfiguration group updated";
                }
                else
                {
                    LogWarn("Group update failed, no function set");
                    co_return module_command::CommandExecutionResult {
                        module_command::Status::FAILURE,
                        "CentralizedConfiguration group update failed, no function set"};
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
                const std::filesystem::path tmpGroupFile = std::filesystem::temp_directory_path() / (groupId + ".conf");
                m_downloadGroupFilesFunction(groupId, tmpGroupFile.string());
                if (!m_validateFileFunction(tmpGroupFile))
                {
                    LogWarn("Validate file failed, invalid group file received: {}", tmpGroupFile.string());
                    co_return module_command::CommandExecutionResult {
                        module_command::Status::FAILURE,
                        "CentralizedConfiguration validate file failed, invalid file received."};
                }

                const std::filesystem::path destGroupFile =
                    std::filesystem::path("/etc/wazuh-agent") / "shared" / (groupId + ".conf");

                try
                {
                    std::filesystem::create_directories(destGroupFile.parent_path());
                    std::filesystem::rename(tmpGroupFile, destGroupFile);
                }
                catch (const std::filesystem::filesystem_error& e)
                {
                    LogWarn("Failed to move file to destination: {}. Error: {}", destGroupFile.string(), e.what());
                    co_return module_command::CommandExecutionResult {module_command::Status::FAILURE,
                                                                      "Failed to move shared file to destination."};
                }
            }

            // TODO apply configuration, remove old group files

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
} // namespace centralized_configuration
