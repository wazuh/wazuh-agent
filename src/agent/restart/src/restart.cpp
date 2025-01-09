#include <restart.hpp>
#include <logger.hpp>
#include <fstream>


namespace restart
{

boost::asio::awaitable<module_command::CommandExecutionResult> Restart::RestartWithSystemd()
{
    LogDebug("Systemctl restarting wazuh agent service.");

    if (std::system("systemctl restart wazuh-agent") == 0)
    {
        co_return module_command::CommandExecutionResult{module_command::Status::IN_PROGRESS, "Systemctl restart execution"};
    }
    else
    {
        LogError("Failed using systemctl.");
        co_return module_command::CommandExecutionResult{module_command::Status::FAILURE, "Systemctl restart failed"};
    }
}

boost::asio::awaitable<module_command::CommandExecutionResult> Restart::RestartWithFork()
{
    pid_t pid = fork();

    if (pid < 0)
    {
        LogError("Fork failed");
    }
    else if (pid == 0)
    {
        // Child process
        StopAgent();

        std::vector<char*> args = get_command_line_args();
        LogDebug("Starting wazuh agent in a new process.");

        if (execve(args[0], args.data(), nullptr) == -1)
        {
            LogError("Failed to spawn new Wazuh agent process.");
        }
    }

    co_return module_command::CommandExecutionResult{module_command::Status::IN_PROGRESS, "Pending restart execution"};

}

// Main command handler for restart
boost::asio::awaitable<module_command::CommandExecutionResult> Restart::HandleRestartCommand()
{

    if (using_systemctl())
    {
        return RestartWithSystemd();
    }
    else
    {
        return RestartWithFork();
    }
}

std::vector<char*> Restart::get_command_line_args()
{
    std::vector<char*> args;
    std::ifstream cmdline_file("/proc/self/cmdline");

    if (!cmdline_file)
    {
        LogError("Failed to open /proc/self/cmdline");
        return args;
    }

    std::string arg;
    while (getline(cmdline_file, arg, '\0'))
    {
        args.push_back(strdup(arg.c_str()));
    }

    args.push_back(nullptr);

    return args;
}

bool Restart::using_systemctl()
{
    return (0 == std::system("which systemctl > /dev/null 2>&1") && nullptr != std::getenv("INVOCATIINVOCATION_IDON_ID"));
}

void Restart::StopAgent()
{
    const int timeout = 30;            // Timeout duration (in seconds) for killing the agent child process
    time_t start_time = time(nullptr); // Record the start time to track the timeout duration

    pid_t pid = getppid();

    // Shutdown Gracefully
    kill(pid, SIGTERM);

    while (true)
    {
        if (kill(pid, 0) != 0)
        {
            LogDebug("Agent process terminated.");
            break;
        }

        if (difftime(time(nullptr), start_time) > timeout)
        {
            LogError("Timeout reached! Forcing agent process termination.");
            kill(pid, SIGKILL);
        }

        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
}

} // namespace restart
