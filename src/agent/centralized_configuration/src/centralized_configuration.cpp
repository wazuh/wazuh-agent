#include <centralized_configuration.hpp>

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
                        co_return module_command::CommandExecutionResult {
                            module_command::Status::FAILURE,
                            "CentralizedConfiguration group set failed, no group list"};

                    groupIds = parameters[0].get<std::vector<std::string>>();
                    messageOnSuccess = "CentralizedConfiguration group set";

                    m_setGroupIdFunction(groupIds);
                }
                else
                {
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
                    co_return module_command::CommandExecutionResult {
                        module_command::Status::FAILURE,
                        "CentralizedConfiguration group update failed, no function set"};
                }
            }
            else
            {
                co_return module_command::CommandExecutionResult {module_command::Status::FAILURE,
                                                                  "CentralizedConfiguration command not recognized"};
            }

            for (const auto& groupId : groupIds)
            {
                m_downloadGroupFilesFunction(groupId, std::filesystem::temp_directory_path().string());
            }

            // TODO validate groupFiles, apply configuration

            co_return module_command::CommandExecutionResult {module_command::Status::SUCCESS, messageOnSuccess};
        }
        catch (const nlohmann::json::exception&)
        {
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
} // namespace centralized_configuration
