#pragma once

#include <string>

namespace unix_daemon
{
    /// @class LockFileHandler class
    /// @brief Handles the creation and deletion of lock files to prevent multiple instances of wazuh agent running
    class LockFileHandler
    {
    public:
        /// @brief Default constructor
        /// @details Creates a lock file when the instance is created
        /// @param lockFilePath The path where the lock file will be created
        LockFileHandler(std::string lockFilePath);

        /// @brief Destructor
        /// @details Removes lock file if it was created on construction
        ~LockFileHandler()
        {
            removeLockFile();
        }

        /// @brief Checks if the lock file has been successfully created
        /// @return True if the lock file is created, false otherwise
        bool isLockFileCreated() const
        {
            return m_lockFileCreated;
        }

        /// @brief Returns the errno from the latest attempt to create/lock the lock-file
        /// @return The errno
        int getErrno() const
        {
            return m_errno;
        }

    private:
        /// @brief Creates the directory path for the lock file
        /// @param path The path for the lock file
        /// @return True if the directory is created, false otherwise
        bool createDirectory(const std::string& path) const;

        /// @brief Creates the lock file
        /// @return True if the lock file is created, false otherwise
        bool createLockFile();

        /// @brief Removes the lock file
        void removeLockFile() const;

        std::string m_lockFilePath;

        /// @brief Holds the errno from the lock-file create/lock attempt
        int m_errno;

        /// @brief Indicates the lock file was created by this instance of the LockFileHandler
        bool m_lockFileCreated;
    };

    /// @brief Gets the status of the daemon
    /// @param configFilePath The path to the configuration file
    /// @return A string indicating whether the daemon is "running" or "stopped"
    std::string GetDaemonStatus(const std::string& configFilePath);

    /// @brief Generates a lock file for the daemon
    /// @param configFilePath The path to the configuration file
    /// @return A LockFileHandler object
    LockFileHandler GenerateLockFile(const std::string& configFilePath);
} // namespace unix_daemon
