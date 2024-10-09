#include <centralized_configuration.hpp>

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

    std::string CentralizedConfiguration::Name() const
    {
        return "CentralizedConfiguration";
    }
} // namespace centralized_configuration
