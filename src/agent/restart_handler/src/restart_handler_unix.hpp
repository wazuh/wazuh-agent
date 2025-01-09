#pragma once
#include <boost/asio/awaitable.hpp>
#include <command_entry.hpp>
#include <restart_handler.hpp>
#include <vector>

namespace restart_handler
{

    /// @brief Checks if systemctl is available and running as a systemd service.
    ///
    /// Determines if systemctl can be used in the current environment.
    /// @return true if systemctl is available and running as a systemd service, otherwise returns false.
    bool UsingSystemctl();

    /// @brief Stops the agent by terminating the child process.
    ///
    /// This function sends a SIGTERM signal to the agent process to stop it. If the agent process
    /// does not stop within a specified timeout period (30 seconds), it forces termination with a SIGKILL signal.
    void StopAgent();

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
