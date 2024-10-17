#include <process_options.hpp>

#include <agent.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <logger.hpp>
#include <windows_service.hpp>

#include <iostream>
#include <vector>

void RestartAgent([[maybe_unused]] const std::string& configPath)
{
    WindowsService::ServiceRestart();
}

void StartAgentDaemon([[maybe_unused]] const std::string& configPath) {}

void StartAgent([[maybe_unused]] const std::string& configPath)
{
    WindowsService::ServiceStart();
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
    std::cout << "     --start                Start wazuh-agent daemon\n";
    std::cout << "     --status               Get wazuh-agent daemon status\n";
    std::cout << "     --stop                 Stop wazuh-agent daemon\n";
    std::cout << "     --restart              Restart wazuh-agent daemon\n";
    std::cout << "     --register-agent       Register wazuh-agent\n";
    std::cout << "     --install-service      Install Windows Service\n";
    std::cout << "     --remove-service       Remove Windows Service\n";
    std::cout << "     --run-service          Used by Windows SCM to run as Service\n";
    std::cout << "     --help                 This help message\n";
}

bool InstallService()
{
    return WindowsService::InstallService();
}

bool RemoveService()
{
    return WindowsService::RemoveService();
}

void SetDispatcherThread()
{
    WindowsService::SetDispatcherThread();
}
