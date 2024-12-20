#include <process_options.hpp>
#include <process_options_unix.hpp>

#include <agent.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <logger.hpp>
#include <unix_daemon.hpp>
#include <fstream>


#include <csignal>
#include <iostream>
#include <thread>
#include <vector>

void StartAgent(const std::string& configFilePath)
{
    bool needsRestart = false;
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
        needsRestart = agent.IsRestartRequired();
    }
    catch (const std::exception& e)
    {
        LogError("Exception thrown in wazuh-agent: {}", e.what());
    }

    lockFileHandler.removeLockFile();

    if ( needsRestart ){
        RestartAgent();
    }
}

void StatusAgent(const std::string& configFilePath)
{
    std::cout << fmt::format("wazuh-agent is {}\n", unix_daemon::GetDaemonStatus(configFilePath));
}


std::vector<const char*> get_command_line_args() {
    std::vector<const char*> args;
    std::ifstream cmdline_file("/proc/self/cmdline");

    if (!cmdline_file) {
        LogError("Failed to open /proc/self/cmdline");
        return args;
    }

    std::string arg;
    while (getline(cmdline_file, arg, '\0')) {
        args.push_back(strdup(arg.c_str()));
    }

    args.push_back(nullptr);

    return args;
}

bool using_systemctl(){
    return (0 == std::system("which systemctl > /dev/null 2>&1") &&
        nullptr != std::getenv("INVOCATION_ID"));
}

void RestartAgent(){

    if ( using_systemctl() )
    {
        LogDebug("Restart: systemctl restarting wazuh agent service.");
        std::system("systemctl restart wazuh-agent");
    }else{
        std::vector<const char*> args = get_command_line_args();
        pid_t pid = fork();

        if (pid == 0) {
            // Child process
            setpgid(0, 0);
            std::this_thread::sleep_for(std::chrono::seconds(1));

            LogDebug("Restart: starting wazuh agent in a new process.");
            if (execve(args[0], const_cast<char* const*>(args.data()), nullptr) == -1) {
                LogError("Failed to spawn new Wazuh agent process.");
            }
            exit(1);
        } else if (pid < 0) {
            LogError("Restart: Fork failed");
            exit(1);
        } else {
            // Parent process
            exit(0);
        }
    }
}
