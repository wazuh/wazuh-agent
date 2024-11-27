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

void RestartAgent([[maybe_unused]] const std::string& configFile) {}

void StartAgent(const std::string& configFile)
{
    unix_daemon::LockFileHandler lockFileHandler = unix_daemon::GenerateLockFile(configFile);

    if (!lockFileHandler.isLockFileCreated())
    {
        std::cout << "wazuh-agent already running\n";
        return;
    }

    LogInfo("Starting wazuh-agent");
    Agent agent(configFile);
    agent.Run();

    lockFileHandler.removeLockFile();
}

void StatusAgent(const std::string& configFile)
{
    std::cout << fmt::format("wazuh-agent is {}\n", unix_daemon::GetDaemonStatus(configFile));
}

void StopAgent() {}

void PrintHelp()
{
    // TO DO
    std::cout << "Wazuh Agent help.\n ";
    std::cout << "Usage: wazuh-agent [options]\n";
    std::cout << "\n";
    std::cout << "Options:\n";
    std::cout << "     " << OPT_STATUS << "             Get wazuh-agent daemon status\n";
    std::cout << "     " << OPT_REGISTER_AGENT << "     Register wazuh-agent\n";
    std::cout << "     " << OPT_CONFIG_FILE << "        Specify the full path of the configuration file (optional).\n";
    std::cout << "                          Can be used when running the application normally or with"
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
