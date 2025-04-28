#pragma once

#include <string>

namespace Utils
{
    /// @brief Executes a command
    /// @param cmd command
    /// @param bufferSize buffer size in bytes
    /// @return command output
    std::string PipeOpen(const std::string& cmd, const size_t bufferSize = 128);
} // namespace Utils
