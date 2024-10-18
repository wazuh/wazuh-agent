#include <process_options.hpp>

#include <agent.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <logger.hpp>
#include <unix_daemon.hpp>

#include <csignal>
#include <iostream>
#include <thread>
#include <vector>

void RestartAgent(const std::string& configPath)
{
    StopAgent();

    std::this_thread::sleep_for(std::chrono::seconds(1)); // NOLINT

    StartAgent(configPath);
}

void StartAgent([[maybe_unused]] const std::string& configPath)
{
    LogInfo("Starting wazuh-agent");

    pid_t pid = unix_daemon::PIDFileHandler::ReadPIDFromFile();
    if (pid != 0)
    {
        LogInfo("wazuh-agent already running");
        return;
    }

    unix_daemon::PIDFileHandler handler = unix_daemon::GeneratePIDFile();
    Agent agent(configPath);
    agent.Run();
}

void StatusAgent()
{
    std::cout << fmt::format("wazuh-agent is {}\n", unix_daemon::GetDaemonStatus());
}

void StopAgent()
{
    LogInfo("Stopping wazuh-agent");
    pid_t pid = unix_daemon::PIDFileHandler::ReadPIDFromFile();
    if (!kill(pid, SIGTERM))
    {
        LogInfo("Wazuh-agent terminated");
    }
    else
    {
        kill(pid, SIGKILL);
        LogInfo("Wazuh-agent killed");
    }
}

void PrintHelp()
{
    // TO DO
    std::cout << "Wazuh Agent help.\n ";
    std::cout << "Usage: wazuh-agent [options]\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "     --run                Start wazuh-agent\n";
    std::cout << "       [--config-path <path>]   Specify configuration file path (optional)\n";
    std::cout << "     --start              Start wazuh-agent daemon\n";
    std::cout << "     --status             Get wazuh-agent daemon status\n";
    std::cout << "     --stop               Stop wazuh-agent daemon\n";
    std::cout << "     --restart            Restart wazuh-agent daemon\n";
    std::cout << "       [--config-path <path>]   Specify configuration file path (optional)\n";
    std::cout << "     --register-agent     Register wazuh-agent\n";
    std::cout << "       [--config-path <path>]   Specify configuration file path (optional)\n";
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
