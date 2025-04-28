#pragma once

#include <string>

namespace Utils
{
    /// @brief Executes a command
    /// @param cmd command
    /// @param bufferSize buffer size in bytes
    /// @return command output
    std::string PipeOpen(const std::string& cmd, const size_t bufferSize = 128);

    /// @brief Executes a command and captures its output.
    ///
    /// Runs a command without using a shell, capturing both stdout and stderr.
    /// The function waits for the process to finish before returning.
    ///
    /// @param cmd The command to execute (no shell features like piping or expansions).
    /// @return The combined stdout and stderr output.
    /// @throw boost::process::process_error if the command fails to start.
    std::string Exec(const std::string& cmd);
} // namespace Utils
