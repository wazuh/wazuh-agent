#pragma once

#include <string_view>
#include <unistd.h>
#include <vector>

namespace unix_daemon
{
    /// @class PIDFileHandler class
    /// @brief Handles the creation, reading, and deletion of PID files for managing process IDs
    class PIDFileHandler
    {
    public:
        /// @brief Default constructor
        /// @details Creates a PID file when the instance is created
        PIDFileHandler();

        /// @brief Destructor
        /// @details Deletes the PID file
        ~PIDFileHandler();

        /// @brief Copy constructor
        PIDFileHandler(const PIDFileHandler&) = delete;

        /// @brief Copy assignment operator
        PIDFileHandler& operator=(const PIDFileHandler&) = delete;

        /// @brief Move constructor
        PIDFileHandler(PIDFileHandler&&) = default;

        /// @brief Move assignment operator
        PIDFileHandler& operator=(PIDFileHandler&&) = default;

        /// @brief Reads the process ID from the PID file
        /// @return The process ID from the PID file
        static pid_t ReadPIDFromFile();

    private:
        /// @brief Creates the directory path for the PID file
        /// @param relativePath The relative path for the PID file
        /// @return True if the directory is created, false otherwise
        bool createDirectory(const std::string_view relativePath) const;

        /// @brief Writes the current process ID to the PID file
        /// @return True if the PID file is written, false otherwise
        bool writePIDFile() const;

        /// @brief Removes the PID file
        /// @return True if the PID file is removed, false otherwise
        bool removePIDFile() const;

        /// @brief Gets the executable path
        /// @return The executable path
        static std::string GetExecutablePath();
    };

    /// @brief Gets the status of the daemon
    /// @return A string indicating whether the daemon is "running" or "stopped"
    std::string GetDaemonStatus();

    /// @brief Generates a PID file for the daemon
    /// @return A PIDFileHandler object
    PIDFileHandler GeneratePIDFile();
} // namespace unix_daemon
