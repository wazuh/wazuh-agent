#pragma once

#include <filesystem_wrapper.hpp>

#include <memory>
#include <string>
#include <sys/types.h>
#include <vector>

namespace os_utils
{
    /// @brief Check if a process with the given PID exists.
    /// @param pid The PID of the process to check.
    /// @return True if the process exists, false otherwise.
    bool PidExists(pid_t pid);

    /// @brief Get the process name of a running process using the ps command.
    /// @param ps The path to the ps command.
    /// @param pid The PID of the process to check.
    /// @return The name of the process if it exists, otherwise an empty string.
    std::string GetProcessName(const std::string& ps, int pid);

    /// @brief Get the list of all running Unix processes.
    /// @param fs A unique pointer to a file system wrapper. Default is a new instance of FileSystemWrapper.
    /// @return A vector of strings containing the names of running processes. Empty if none or error found.
    std::vector<std::string>
    GetRunningProcesses(std::unique_ptr<IFileSystemWrapper> fs = std::make_unique<file_system::FileSystemWrapper>());
} // namespace os_utils
