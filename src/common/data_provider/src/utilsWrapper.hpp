#pragma once

#include <string>

class UtilsWrapper final
{
public:
    /// @brief Executes a command
    /// @param cmd command
    /// @param bufferSize buffer size in bytes
    /// @return command output
    static std::string exec(const std::string& cmd, const size_t bufferSize = 128);
};
