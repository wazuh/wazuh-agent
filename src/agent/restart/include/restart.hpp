#pragma once
#include <module_command/command_entry.hpp>

#include <boost/asio/awaitable.hpp>

#include <string>
#include <vector>

namespace restart
{
    /// @brief Restart class.
    class Restart
    {
    public:
        explicit Restart();

        static boost::asio::awaitable<module_command::CommandExecutionResult> HandleRestartCommand();

        /// @brief Restart the agent
        ///
        /// After the agent stops, initiates a start but in a new process.
        void RestartAgent();

    private:
        /// @brief Retrieves command-line arguments for `execve()`.
        ///
        /// Parses `/proc/self/cmdline` to extract arguments.
        /// Returns a vector of null-terminated strings.
        static std::vector<char*> get_command_line_args();

        /// @brief Checks if systemctl is available and running as a systemd service.
        ///
        /// Determines if systemctl can be used in the current environment.
        static bool using_systemctl();

        /// @brief Stops the agent by terminating the child process.
        ///
        /// This function sends a SIGTERM signal to the agent process to stop it. If the agent process
        /// does not stop within a specified timeout period (30 seconds), it forces termination with a SIGKILL signal.
        ///
        /// @param pid The process ID of the agent to stop.
        static void StopAgent();

        static boost::asio::awaitable<module_command::CommandExecutionResult> RestartWithFork();
        static boost::asio::awaitable<module_command::CommandExecutionResult> RestartWithSystemd();


    };
} // namespace restart
