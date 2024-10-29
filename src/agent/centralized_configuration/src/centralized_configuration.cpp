#include <centralized_configuration.hpp>

#include <nlohmann/json.hpp>

#include <filesystem>

namespace centralized_configuration
{
    boost::asio::awaitable<module_command::CommandExecutionResult> CentralizedConfiguration::ExecuteCommand(
        const std::string command,       // NOLINT(performance-unnecessary-value-param)
        const nlohmann::json parameters) // NOLINT(performance-unnecessary-value-param)
    {
        try
        {

            if (command == "set-group")
            {
                if (m_setGroupIdFunction && m_downloadGroupFilesFunction)
                {
                    if (parameters.empty())
                        co_return module_command::CommandExecutionResult {
                            module_command::Status::FAILURE,
                            "CentralizedConfiguration group set failed, no group list"};

                    const auto groupIds = parameters[0].get<std::vector<std::string>>();

                    m_setGroupIdFunction(groupIds);

                    for (const auto& groupId : groupIds)
                    {
                        m_downloadGroupFilesFunction(groupId, std::filesystem::temp_directory_path().string());
                    }

                    // TODO validate groupFiles, apply configuration

                    co_return module_command::CommandExecutionResult {module_command::Status::SUCCESS,
                                                                      "CentralizedConfiguration group set"};
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
                    const auto groupIds = m_getGroupIdFunction();

                    for (const auto& groupId : groupIds)
                    {
                        m_downloadGroupFilesFunction(groupId, std::filesystem::temp_directory_path().string());
                    }

                    // TODO validate groupFiles, apply configuration

                    co_return module_command::CommandExecutionResult {module_command::Status::SUCCESS,
                                                                      "CentralizedConfiguration group updated"};
                }
                else
                {
                    co_return module_command::CommandExecutionResult {
                        module_command::Status::FAILURE, "CentralizedConfiguration group set failed, no function set"};
                }
            }
        }
        catch (const nlohmann::json::exception&)
        {
            co_return module_command::CommandExecutionResult {
                module_command::Status::FAILURE, "CentralizedConfiguration error while parsing parameters"};
        }

        co_return module_command::CommandExecutionResult {module_command::Status::FAILURE,
                                                          "CentralizedConfiguration command not recognized"};
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
