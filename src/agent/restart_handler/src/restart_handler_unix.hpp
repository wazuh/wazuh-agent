#pragma once
#include <boost/asio/awaitable.hpp>
#include <restart_handler.hpp>
#include <vector>

namespace restart_handler
{

    /// @brief Checks if systemctl is available and running as a systemd service.
    ///
    /// Determines if systemctl can be used in the current environment.
    /// @return true if systemctl is available and running as a systemd service, otherwise returns false.
    bool UsingSystemctl();

    /// @brief Stops the agent by terminating its process.
    ///
    /// This function sends a SIGTERM signal to the agent process to request its termination. If the agent
    /// process does not stop within the specified timeout period (30 seconds), a SIGKILL signal is sent to force
    /// termination.
    ///
    /// @param pid The process ID of the agent to be stopped.
    /// @param timeout The maximum time (in seconds) to wait for the agent to stop before forcing termination.
    void StopAgent(const pid_t pid, const int timeout);

    /// @brief Restarts the module by forking a new process.
    ///
    /// This function restarts the module by creating a new child process.
    ///
    /// @return A boost::asio::awaitable containing the result of the command execution.
    boost::asio::awaitable<module_command::CommandExecutionResult> RestartWithFork();

    /// @brief Restarts the module using systemd service management.
    ///
    /// This function restarts the module via systemd, ensuring the module is properly restarted using
    /// system service management mechanisms.
    ///
    /// @return A boost::asio::awaitable containing the result of the command execution.
    boost::asio::awaitable<module_command::CommandExecutionResult> RestartWithSystemd();

} // namespace restart_handler
