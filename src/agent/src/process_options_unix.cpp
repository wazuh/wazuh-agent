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
