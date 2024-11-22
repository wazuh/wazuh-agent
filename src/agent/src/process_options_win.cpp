#include <process_options.hpp>

#include <agent.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <logger.hpp>
#include <windows_service.hpp>

#include <iostream>
#include <vector>

void RestartAgent(const std::string& configFile)
{
    WindowsService::ServiceRestart(configFile);
}

void StartAgent(const std::string& configFile)
{
    WindowsService::ServiceStart(configFile);
}

void StatusAgent()
{
    WindowsService::ServiceStatus();
}

void StopAgent()
{
    WindowsService::ServiceStop();
}

void PrintHelp()
{
    LogInfo("Wazuh Agent help.");

    // TO DO
    std::cout << "Usage: wazuh-agent [options]\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "     " << OPT_STATUS << "               Get wazuh-agent daemon status\n";
    std::cout << "     " << OPT_STOP << "                 Stop wazuh-agent daemon\n";
    std::cout << "     " << OPT_RESTART << "              Restart wazuh-agent daemon\n";
    std::cout << "     " << OPT_REGISTER_AGENT << "       Register wazuh-agent\n";
    std::cout << "     " << OPT_INSTALL_SERVICE << "      Install Windows Service\n";
    std::cout << "     " << OPT_REMOVE_SERVICE << "       Remove Windows Service\n";
    std::cout << "     " << OPT_HELP << "                 This help message\n";
}

bool InstallService()
{
    windows_api_facade::WindowsApiFacade windowsApiFacade;
    return WindowsService::InstallService(windowsApiFacade);
}

bool RemoveService()
{
    windows_api_facade::WindowsApiFacade windowsApiFacade;
    return WindowsService::RemoveService(windowsApiFacade);
}

void SetDispatcherThread()
{
    WindowsService::SetDispatcherThread();
}
