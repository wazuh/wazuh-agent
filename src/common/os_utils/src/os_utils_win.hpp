#pragma once

#include <windows.h>

#include <functional>
#include <string>
#include <tlhelp32.h>
#include <vector>

namespace os_utils
{
    struct ProcessSnapshotAPI
    {
        std::function<HANDLE(DWORD, DWORD)> CreateSnapshot = CreateToolhelp32Snapshot;
        std::function<BOOL(HANDLE, LPPROCESSENTRY32)> ProcessFirst = Process32First;
        std::function<BOOL(HANDLE, LPPROCESSENTRY32)> ProcessNext = Process32Next;
    };

    /// @brief Get the list of all running Windows processes.
    /// @param api A struct containing function pointers for process snapshot API calls.
    /// @return A vector of strings containing the names of running processes. Empty if none or error found.
    std::vector<std::string> GetRunningProcesses(const ProcessSnapshotAPI& api = {});
} // namespace os_utils
