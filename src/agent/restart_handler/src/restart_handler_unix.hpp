#pragma once

#include <restart_handler.hpp>

namespace restart_handler
{
    /// @brief Stops the agent by terminating its process.
    ///
    /// This function sends a SIGTERM signal to the agent process to request its termination. If the agent
    /// process does not stop within the specified timeout period (30 seconds), a SIGKILL signal is sent to force
    /// termination.
    ///
    /// @param pid The process ID of the agent to be stopped.
    /// @param timeout The maximum time (in seconds) to wait for the agent to stop before forcing termination.
    void StopAgent(const pid_t pid, const int timeout);
} // namespace restart_handler
