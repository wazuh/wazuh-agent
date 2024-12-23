#include <process_options.hpp>
#include <process_options_unix.hpp>

#include <agent.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>
#include <logger.hpp>
#include <fstream>
#include <ctime>

#include <csignal>
#include <iostream>
#include <thread>
#include <vector>

#include <sys/wait.h>


// Flag to signal that SIGUSR1 was received
volatile sig_atomic_t signal_received = 0;

// Signal handler for SIGUSR1
void sigusr1_handler(int signal) {
    if (signal == SIGUSR1) {
        LogDebug("Received SIGUSR1: Restarting child agent process...");
        signal_received = SIGUSR1;  // Set flag to indicate restart needed
    }
}

// Signal handler for SIGCHLD
void sigchld_handler(int signal) {
    if (signal == SIGCHLD) {
        LogDebug("Received SIGCHLD: Child agent process terminated." );
    }
}


// Signal handler for SIGTERM
void sigterm_handler(int signal) {
    if (signal == SIGTERM) {
        LogDebug("Received SIGTERM: Stop the agent process...");
        signal_received = SIGTERM;  // Set flag to indicate restart needed
    }
}

void StartAgent(const std::string& configFilePath)
{
    // Set up signal handlers
    struct sigaction sa_usr1, sa_chld, sa_term;

    sa_usr1.sa_handler = sigusr1_handler;
    sa_usr1.sa_flags = 0;
    sigaction(SIGUSR1, &sa_usr1, nullptr);

    sa_chld.sa_handler = sigchld_handler;
    sa_chld.sa_flags = SA_NOCLDSTOP;  // Avoid receiving SIGCHLD for stopped children
    sigaction(SIGCHLD, &sa_chld, nullptr);

    // Set up SIGTERM handler
    sa_term.sa_handler = sigterm_handler;
    sa_term.sa_flags = 0;
    sigaction(SIGTERM, &sa_term, nullptr);


    unix_daemon::LockFileHandler lockFileHandler = unix_daemon::GenerateLockFile(configFilePath);


    pid_t pid = fork();

    if (pid < 0) {
        LogError("Restart: Fork failed");
        exit(1);
    } else if (pid == 0) {
        // Child process: Run the agent
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

        lockFileHandler.removeLockFile();

        exit(0);
    } else {
        // Parent process - Monitoring Agent, take care of slefrestart
        pause();  // Suspend parent until a signal is received
        std::cout << "Wait to the child process ends." << std::endl;

        // Stop Agent
        if ( signal_received == SIGTERM ){
            std::cout << "Received SIGTERM, terminating child process..." << std::endl;
            kill(pid, SIGTERM); // Send SIGTERM to the child process
            waitpid(pid, nullptr, 0); // Wait for the child to terminate
        }

        // Self-restart agent
        if (signal_received == SIGUSR1) {
            if ( using_systemctl() ) {
                LogDebug("Restart: systemctl restarting wazuh agent service.");
                std::system("systemctl restart wazuh-agent");
            } else {
                StopAgent(pid, &lockFileHandler);

                std::vector<const char*> args = get_command_line_args();
                LogDebug("Restart: starting wazuh agent in a new process.");
                if (execve(args[0], const_cast<char* const*>(args.data()), nullptr) == -1) {
                    LogError("Failed to spawn new Wazuh agent process.");
                }
            }
        }
        exit(0); // Exit the parent process
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


void StopAgent(pid_t pid, unix_daemon::LockFileHandler *lockFileHandler){

    int status;
    pid_t result;

    const int timeout = 30; // Timeout duration (in seconds) for killing the agent child process
    time_t start_time = time(nullptr); // Record the start time to track the timeout duration

    // Initiate the process termination by sending SIGTERM
    kill(pid, SIGTERM);

    while(true){
        result = waitpid(pid, &status, WNOHANG);  // Non-blocking check for agent process status

        if (result == pid) {
            LogDebug("Agent process terminated.");
            break;
        }

        if (difftime(time(nullptr), start_time) > timeout) {
            LogError("Timeout reached! Forcing agent process termination.");
            kill(pid, SIGKILL);
            lockFileHandler->removeLockFile();
        }

        // Sleep for a short time before checking again
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}