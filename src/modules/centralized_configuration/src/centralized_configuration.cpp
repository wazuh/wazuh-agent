#include <centralized_configuration.hpp>

#include <module_command/command_entry.hpp>

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
    Co_CommandExecutionResult CentralizedConfiguration::ExecuteCommand(const std::string)
    {
        co_return module_command::CommandExecutionResult{
            module_command::Status::FAILURE,
            "Not yet implemented"
        };
    }

    std::string CentralizedConfiguration::Name() const
    {
        return "CentralizedConfiguration";
    }
} // namespace centralized_configuration
