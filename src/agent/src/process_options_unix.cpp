#include <process_options.hpp>
// #include <systemd/sd-daemon.h>
#include <unistd.h>
#include <agent.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <logger.hpp>
#include <unix_daemon.hpp>

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
    Agent agent(configFilePath);
    agent.Run();

    lockFileHandler.removeLockFile();
}

void StatusAgent(const std::string& configFilePath)
{
    std::cout << fmt::format("wazuh-agent is {}\n", unix_daemon::GetDaemonStatus(configFilePath));
}

void StopAgent(const std::string& configFilePath)
{
    unix_daemon::LockFileHandler lockFileHandler = unix_daemon::GenerateLockFile(configFilePath);

    if (lockFileHandler.isLockFileCreated())
    {
        LogInfo("wazuh-agent is not running");
        return;
    }

    pid_t pid = lockFileHandler.ReadPIDFromFile();

    if (pid < 0)
    {
        LogError("Error reading pid file");
        return;
    }

    if (!kill(pid, SIGTERM))
    {
        LogInfo("wazuh-agent stopped successfully");
    }

    lockFileHandler.removeLockFile();
}

void RestartAgent(const std::string& configFile)
{
    LogInfo("Restarting wazuh-agent...");
    StopAgent(configFile);
    int timeoutSeconds = 20;
    auto startTime = std::chrono::steady_clock::now();

    // Periodically check if the agent has stopped
    while ( unix_daemon::IsDaemonRunning(configFile) )   {
        // Check elapsed time
        std::cout << unix_daemon::GetDaemonStatus(configFile)  << std::endl;
        auto elapsed = std::chrono::steady_clock::now() - startTime;
        if (std::chrono::duration_cast<std::chrono::seconds>(elapsed).count() > timeoutSeconds) {
            LogError("Timeout reached while stopping wazuh-agent.");
            return ;
        }

        // Sleep briefly before checking again
        std::this_thread::sleep_for(std::chrono::milliseconds(100));
        LogInfo("Waiting wazuh-agent... be stoped.");
    }

    StartAgent(configFile);
    LogInfo("Restarting wazuh-agent... Done.");
}