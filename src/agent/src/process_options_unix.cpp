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

void RestartAgent(const std::string& configFile)
{
    StopAgent();

    std::this_thread::sleep_for(std::chrono::seconds(1)); // NOLINT

    StartAgent(configFile);
}

void StartAgent([[maybe_unused]] const std::string& configFile)
{
    pid_t pid = unix_daemon::PIDFileHandler::ReadPIDFromFile();
    if (pid != 0)
    {
        std::cout << "wazuh-agent already running\n";
        return;
    }
    LogInfo("Starting wazuh-agent");
    unix_daemon::PIDFileHandler handler = unix_daemon::GeneratePIDFile();
    Agent agent(configFile);
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
    std::cout << "     " << OPT_RUN << "                Start wazuh-agent\n";
    std::cout << "     " << OPT_START << "              Start wazuh-agent daemon\n";
    std::cout << "     " << OPT_STATUS << "             Get wazuh-agent daemon status\n";
    std::cout << "     " << OPT_STOP << "               Stop wazuh-agent daemon\n";
    std::cout << "     " << OPT_RESTART << "            Restart wazuh-agent daemon\n";
    std::cout << "     " << OPT_REGISTER_AGENT << "     Register wazuh-agent\n";
    std::cout << "     " << OPT_CONFIG_FILE << "        Specify the full path of the configuration file (optional).\n";
    std::cout << "                          Used with " << OPT_RUN << ", " << OPT_RESTART << ", or "
              << OPT_REGISTER_AGENT << ".\n";
    std::cout << "     " << OPT_HELP << "               This help message\n";
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
