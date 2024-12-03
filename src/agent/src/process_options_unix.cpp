#include <process_options.hpp>
#include <systemd/sd-daemon.h>
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

// std::string GetParentProcessName() {
//     pid_t ppid = getppid();
//     std::string procPath = "/proc/" + std::to_string(ppid) + "/comm";
//     std::ifstream procFile(procPath);
//     if (!procFile.is_open()) {
//         return "";
//     }

//     std::string parentName;
//     std::getline(procFile, parentName);
//     procFile.close();
//     return parentName;
// }


void StartAgent(const std::string& configFilePath)
{
    unix_daemon::LockFileHandler lockFileHandler = unix_daemon::GenerateLockFile(configFilePath);

    if (!lockFileHandler.isLockFileCreated())
    {
        std::cout << "wazuh-agent already running\n";
        return;
    }

    // if ( "systemctl" != GetParentProcessName() ) {
    //     // Notify systemd that the service is running
    //     std::cout << fmt::format("Notify systemctl that is running {}\n", unix_daemon::GetDaemonStatus(configFilePath));
    //     sd_notify(0, "READY=1");
    //     sd_notify(0, "STATUS=Service is running");
	// }

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
        std::cout << "wazuh-agent is not running\n";
        return;
    }

    pid_t pid = lockFileHandler.ReadPIDFromFile();

    if (pid < 0)
    {
        std::cout << "Error reading pid file\n";
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
    StopAgent(configFile);

    std::this_thread::sleep_for(std::chrono::seconds(1)); // NOLINT

    StartAgent(configFile);
}