#include <process_options.hpp>

#include <agent.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <logger.hpp>
#include <unix_daemon.hpp>

#include <iostream>
#include <vector>

void RestartAgent()
{
    StopAgent();
    StartAgent();
}

void StartAgent()
{
    LogInfo("Starting wazuh-agent");

    unix_daemon::PIDFileHandler handler = unix_daemon::GeneratePIDFile();

    Agent agent;
    agent.Run();
}

void StatusAgent()
{
    std::cout << fmt::format("wazuh-agent is {}\n", unix_daemon::GetDaemonStatus());
}

void StopAgent()
{
    LogInfo("Stopping wazuh-agent");
}

void InstallService([[maybe_unused]] const std::string& exePath) {}

void RemoveService() {}

void SetDispatcherThread() {}
