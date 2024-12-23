#pragma once

#include <unix_daemon.hpp>

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

/// @brief Stops the agent by terminating the child process.
///
/// This function sends a SIGTERM signal to the agent process to stop it. If the agent process
/// does not stop within a specified timeout period (30 seconds), it forces termination with a SIGKILL signal.
///
/// @param pid The process ID of the agent to stop.
/// @param lockFileHandler Pointer to the lock file handler used to remove the lock file if necessary.
void StopAgent(pid_t pid, unix_daemon::LockFileHandler* lockFileHandler);

/// @brief Handles SIGUSR1 signal
///
/// This function is called when the SIGUSR1 signal is received.
/// It handles the self-restart actions triggered by this signal.
void sigusr1_handler(int signal);

/// @brief Handles SIGCHLD signal
///
/// This function is invoked when a child process terminates.
/// It manages the termination of child processes by handling the SIGCHLD signal.
void sigchld_handler(int signal);

/// @brief Handles SIGTERM signal
///
/// This function is invoked when the process receives a SIGTERM signal. It is typically used
/// for gracefully shutting down the process.
///
/// @param signal The signal number (SIGTERM) that triggered this handler.
void sigterm_handler(int signal);
