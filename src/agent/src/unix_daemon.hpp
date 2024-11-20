#pragma once

#include <string_view>
#include <unistd.h>
#include <vector>

namespace unix_daemon
{
    /// @class LockFileHandler class
    /// @brief Handles the creation and deletion of lock files to prevent multiple instances of wazuh agent running
    class LockFileHandler
    {
    public:
        /// @brief Default constructor
        /// @details Creates a lock file when the instance is created
        LockFileHandler();

        /// @brief Checks if the lock file has been successfully created
        /// @return True if the lock file is created, false otherwise
        bool isLockFileCreated() const
        {
            return m_lockFileCreated;
        }

        /// @brief Removes the lock file
        /// @return True if the file is removed, false otherwise
        bool removeLockFile() const;

    private:
        /// @brief Creates the directory path for the lock file
        /// @param path The path for the lock file
        /// @return True if the directory is created, false otherwise
        bool createDirectory(const std::string& path) const;

        /// @brief Creates the lock file
        /// @return True if the lock file is created, false otherwise
        bool createLockFile();

        /// @brief Gets the executable path
        /// @return The executable path
        static std::string GetExecutablePath();

        bool m_lockFileCreated;
    };

    /// @brief Gets the status of the daemon
    /// @return A string indicating whether the daemon is "running" or "stopped"
    std::string GetDaemonStatus();

    /// @brief Generates a lock file for the daemon
    /// @return A LockFileHandler object
    LockFileHandler GenerateLockFile();
} // namespace unix_daemon
