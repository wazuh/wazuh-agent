#include <agent_runner.hpp>

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
    cmdParser.add_options()
        (OPT_INSTALL_SERVICE, OPT_INSTALL_SERVICE_DESC)
        (OPT_REMOVE_SERVICE, OPT_REMOVE_SERVICE_DESC)
        (OPT_RUN_SERVICE, OPT_RUN_SERVICE_DESC);
    // clang-format on
}

std::optional<int> AgentRunner::HandlePlatformSpecificOptions() const
{
    if (validOptions.count(OPT_INSTALL_SERVICE))
    {
        if (!InstallService())
        {
            return 1;
        }
    }
    else if (validOptions.count(OPT_REMOVE_SERVICE))
    {
        if (!RemoveService())
        {
            return 1;
        }
    }
    else if (validOptions.count(OPT_RUN_SERVICE))
    {
        SetDispatcherThread();
    }
    else
    {
        return std::nullopt;
    }
    return 0;
}
