#include <centralized_configuration.hpp>

#include <module_command/command_entry.hpp>

#include <nlohmann/json.hpp>

#include <filesystem>

namespace centralized_configuration
{
    void CentralizedConfiguration::Start() const
    {
    }

    void CentralizedConfiguration::Setup(const configuration::ConfigurationParser&) const
    {
    }

    void CentralizedConfiguration::Stop() const
    {
    }

    // NOLINTNEXTLINE(performance-unnecessary-value-param)
    Co_CommandExecutionResult CentralizedConfiguration::ExecuteCommand(const std::string command)
    {
        try
        {
            const auto commnandAsJson = nlohmann::json::parse(command);

            if (commnandAsJson["command"] == "set-group")
            {
                if (m_setGroupIdFunction && m_downloadGroupFilesFunction)
                {
                    const auto groupIds = commnandAsJson["groups"].get<std::vector<std::string>>();

                    m_setGroupIdFunction(groupIds);

                    for (const auto& groupId : groupIds)
                    {
                        m_downloadGroupFilesFunction(
                            groupId,
                            std::filesystem::temp_directory_path().string()
                        );
                    }

                    // TODO validate groupFiles, apply configuration

                    co_return module_command::CommandExecutionResult{
                        module_command::Status::SUCCESS,
                        "CentralizedConfiguration group set"
                    };
                }
                else
                {
                    co_return module_command::CommandExecutionResult{
                        module_command::Status::FAILURE,
                        "CentralizedConfiguration group set failed, no function set"
                    };
                }
            }
            else if (commnandAsJson["command"] == "update-group")
            {
                co_return module_command::CommandExecutionResult{
                    module_command::Status::SUCCESS,
                    "CentralizedConfiguration group updated"
                };
            }
        }
        catch (const nlohmann::json::exception&)
        {
            co_return module_command::CommandExecutionResult{
                module_command::Status::FAILURE,
                "CentralizedConfiguration error while parsing command"
            };
        }

        co_return module_command::CommandExecutionResult{
            module_command::Status::FAILURE,
            "CentralizedConfiguration command not recognized"
        };
    }

    std::string CentralizedConfiguration::Name() const
    {
        return "CentralizedConfiguration";
    }

    void CentralizedConfiguration::SetGroupIdFunction(SetGroupIdFunctionType setGroupIdFunction)
    {
        m_setGroupIdFunction = std::move(setGroupIdFunction);
    }

    void CentralizedConfiguration::SetDownloadGroupFilesFunction(DownloadGroupFilesFunctionType downloadGroupFilesFunction)
    {
        m_downloadGroupFilesFunction = std::move(downloadGroupFilesFunction);
    }
} // namespace centralized_configuration
