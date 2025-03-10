#pragma once

#include <ifilesystem.hpp>

#include <string>

namespace instance_handler
{
    /// @class InstanceHandler
    /// @brief Handles the acquisition and release of instance locks to prevent multiple instances of wazuh agent
    /// running
    class InstanceHandler
    {
    public:
        /// @brief InstanceHandler constructor
        /// @details Creates a instance lock when the instance is created (lock file for linux/macOS, named mutex for
        /// Windows).
        /// @param lockFilePath The path where the lock file will be created (linux/macOS only). Unused in Windows, use
        /// "".
        /// @param fileSystemWrapper An optional filesystem wrapper. If nullptr, it will use FileSystemWrapper. Unused
        /// in Windows.
        explicit InstanceHandler(std::string lockFilePath, std::shared_ptr<IFileSystem> fileSystemWrapper = nullptr);

        /// @brief Destructor
        /// @details Removes lock file if it was created on construction (linux/macOS only). Windows implementation uses
        /// a named mutex which is released on destruction by the OS.
        ~InstanceHandler();

        /// @brief Checks if the instance lock has been successfully acquired
        /// @return True if the lock is acquired, false otherwise
        bool isLockAcquired() const;

        /// @brief Returns the errno from the latest attempt to create/lock the lock-file or mutex
        /// @return The errno
        int getErrno() const;

    private:
        /// @brief Gets a lock on the either the lock file or the mutex
        /// @return True if the lock is aqcuired, false otherwise
        bool getInstanceLock();

        /// @brief Releases the instance lock on the lock file or mutex
        void releaseInstanceLock() const;

        std::string m_lockFilePath;

        /// @brief Holds the errno from the lock-file create/lock attempt
        int m_errno;

        /// @brief Member to interact with the file system.
        std::shared_ptr<IFileSystem> m_fileSystemWrapper;

        /// @brief Indicates the lock has been acquired by this instance of the InstanceHandler
        bool m_lockAcquired;
    };

    /// @brief Gets the status of the daemon
    /// @param configFilePath The path to the configuration file
    /// @return A string indicating whether the daemon is "running" or "stopped"
    std::string GetAgentStatus(const std::string& configFilePath);

    /// @brief Generates an instance handler for the daemon
    /// @param configFilePath The path to the configuration file
    /// @return An InstanceHandler object
    InstanceHandler GetInstanceHandler(const std::string& configFilePath);
} // namespace instance_handler
