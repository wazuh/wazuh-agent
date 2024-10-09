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

void StartAgentDaemon()
{
    unix_daemon::PIDFileHandler handler = unix_daemon::GeneratePIDFile();
    StartAgent();
}

void StartAgent()
{
    LogInfo("Starting wazuh-agent");

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

void PrintHelp()
{
    LogInfo("Wazuh Agent help.");

    // TO DO
    std::cout << "Usage: wazuh-agent [options]\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "     --run                Start wazuh-agent\n";
    std::cout << "     --start              Start wazuh-agent daemon\n";
    std::cout << "     --status             Get wazuh-agent daemon status\n";
    std::cout << "     --stop               Stop wazuh-agent daemon\n";
    std::cout << "     --restart            Restart wazuh-agent daemon\n";
    std::cout << "     --register-agent     Register wazuh-agent\n";
    std::cout << "     --help               This help message\n";
}

bool InstallService()
{
    return true;
}

bool RemoveService()
{
    return true;
}

void SetDispatcherThread() {}
