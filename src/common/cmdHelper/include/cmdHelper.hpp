#pragma once

#include <string>

namespace Utils
{
    struct ExecResult
    {
        std::string StdOut;
        std::string StdErr;
        int ExitCode;
    };

    /// @brief Executes a command
    /// @param cmd command
    /// @param bufferSize buffer size in bytes
    /// @return command output
    std::string PipeOpen(const std::string& cmd, const size_t bufferSize = 128);

    /// @brief Executes a command and captures its output.
    ///
    /// Runs a command without using a shell, capturing both StdOut and StdErr.
    /// The function waits for the process to finish before returning.
    ///
    /// @param cmd The command to execute (no shell features like piping or expansions).
    /// @return A ExecResult containing the command's StdOut, StdErr, and ExitCode.
    /// @throw boost::process::process_error if the command fails to start.
    ExecResult Exec(const std::string& cmd);
} // namespace Utils
