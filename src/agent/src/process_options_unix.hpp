#pragma once

/// @brief Retrieves command-line arguments for `execve()`.
///
/// Parses `/proc/self/cmdline` to extract arguments.
/// Returns a vector of null-terminated strings.
static std::vector<const char*> get_command_line_args();

/// @brief Checks if systemctl is available and running as a systemd service.
///
/// Determines if systemctl can be used in the current environment.
static bool using_systemctl();

/// @brief Restart the agent
///
/// After the agent stops, initiates a start but in a new process.
void RestartAgent();
