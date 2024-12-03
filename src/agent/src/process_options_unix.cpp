#include <process_options.hpp>
#include <unistd.h>
#include <agent.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <logger.hpp>
#include <unix_daemon.hpp>
#include <sys/types.h>

#include <csignal>
#include <iostream>
#include <thread>
#include <vector>
#include <chrono>
#include <string>



void StartAgent(const std::string& configFilePath, const char** argv)
{
    unix_daemon::LockFileHandler lockFileHandler = unix_daemon::GenerateLockFile(configFilePath);

    if (!lockFileHandler.isLockFileCreated())
    {
        std::cout << "wazuh-agent already running\n";
        return;
    }

    LogInfo("Starting wazuh-agent");

    try
    {
        Agent agent(configFilePath, argv);
        agent.Run();
    }
    catch (const std::exception& e)
    {
        LogError("Exception thrown in wazuh-agent: {}", e.what());
    }

    lockFileHandler.removeLockFile();
}

void StatusAgent(const std::string& configFilePath)
{
    std::cout << fmt::format("wazuh-agent is {}\n", unix_daemon::GetDaemonStatus(configFilePath));
}
