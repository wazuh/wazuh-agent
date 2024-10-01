#include <process_options.hpp>

#include <agent.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <logger.hpp>
#include <windows_service.hpp>

#include <vector>

void RestartAgent() {}

void StartAgent()
{
    LogInfo("Starting Wazuh Agent.");
}

void StatusAgent() {}

void StopAgent()
{
    LogInfo("Stopping Wazuh Agent.");
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
