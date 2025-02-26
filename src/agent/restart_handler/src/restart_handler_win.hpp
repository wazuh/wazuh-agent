#pragma once

#include <restart_handler.hpp>

#include <string>

namespace restart_handler
{
    /// @brief Gets the path to the Windows PowerShell executable.
    ///
    /// Uses the Windows API to get the path to the Windows PowerShell executable in the system.
    ///
    /// @return The full path to the Windows PowerShell executable.
    std::string GetPowerShellPath();
} // namespace restart_handler
