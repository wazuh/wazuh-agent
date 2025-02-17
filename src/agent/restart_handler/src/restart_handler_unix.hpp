#pragma once

#include <boost/asio/awaitable.hpp>

#include <restart_handler.hpp>
#include <vector>

namespace restart_handler
{
    /// @brief Checks if the Agent is running as a service
    ///
    /// @details Looks for indications that the Agent was started by systemd, launchd or SCM
    /// @return true if the Agent is running as a service, otherwise returns false.
    bool RunningAsService();

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

    /// @brief Restarts the module as a service
    ///
    /// This function restarts the module, ensuring the module is properly restarted using
    /// system service management mechanisms.
    ///
    /// @return A boost::asio::awaitable containing the result of the command execution.
    boost::asio::awaitable<module_command::CommandExecutionResult> RestartService();
} // namespace restart_handler
