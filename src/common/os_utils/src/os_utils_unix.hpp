#pragma once

#include <filesystem_wrapper.hpp>

#include <string>
#include <vector>

namespace os_utils
{
    /// @brief Check if a process with the given PID exists.
    /// @param pid The PID of the process to check.
    /// @return True if the process exists, false otherwise.
    bool PidExists(pid_t pid);

    /// @brief Get the process name of a running process.
    /// @param pid The PID of the process to check.
    /// @return The name of the process if it exists, otherwise an empty string.
    std::string GetProcessName(int pid);

    /// @brief Get the list of all running Unix processes.
    /// @return A vector of strings containing the names of running processes. Empty if none or error found.
    std::vector<std::string> GetRunningProcesses();
} // namespace os_utils
