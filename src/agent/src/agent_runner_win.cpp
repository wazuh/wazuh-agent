#include <agent_runner.hpp>

#include <windows_service.hpp>

namespace
{
    /// Command-line options
    static const auto OPT_INSTALL_SERVICE {"install-service"};
    static const auto OPT_INSTALL_SERVICE_DESC {"Use this option to install Wazuh as a Windows service"};
    static const auto OPT_REMOVE_SERVICE {"remove-service"};
    static const auto OPT_REMOVE_SERVICE_DESC {"Use this option to remove Wazuh Windows service"};
    static const auto OPT_RUN_SERVICE {"run-service"};
    static const auto OPT_RUN_SERVICE_DESC {"Use this option to run Wazuh as a Windows service"};
} // namespace

void AgentRunner::AddPlatformSpecificOptions()
{
    // clang-format off
    m_generalOptions.add_options()
        (OPT_INSTALL_SERVICE, OPT_INSTALL_SERVICE_DESC)
        (OPT_REMOVE_SERVICE, OPT_REMOVE_SERVICE_DESC)
        (OPT_RUN_SERVICE, OPT_RUN_SERVICE_DESC);
    // clang-format on
}

std::optional<int> AgentRunner::HandlePlatformSpecificOptions() const
{
    if (m_options.count(OPT_INSTALL_SERVICE))
    {
        windows_api_facade::WindowsApiFacade windowsApiFacade;
        return windows_service::InstallService(windowsApiFacade) ? 0 : 1;
    }
    else if (m_options.count(OPT_REMOVE_SERVICE))
    {
        windows_api_facade::WindowsApiFacade windowsApiFacade;
        return windows_service::RemoveService(windowsApiFacade) ? 0 : 1;
    }
    else if (m_options.count(OPT_RUN_SERVICE))
    {
        windows_service::SetDispatcherThread();
    }
    return std::nullopt;
}
