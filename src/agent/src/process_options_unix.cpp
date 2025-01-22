#include <process_options.hpp>

#include <agent.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <instance_handler.hpp>
#include <logger.hpp>

#include <csignal>
#include <iostream>
#include <thread>
#include <vector>

void StartAgent(const std::string& configFilePath)
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
        Agent agent(configFilePath);
        agent.Run();
    }
    catch (const std::exception& e)
    {
        LogError("Exception thrown in wazuh-agent: {}", e.what());
    }
}

void StatusAgent(const std::string& configFilePath)
{
    std::cout << fmt::format("wazuh-agent status: {}\n", unix_daemon::GetDaemonStatus(configFilePath));
}
