#pragma once

#include <string_view>
#include <unistd.h>
#include <vector>

namespace unix_daemon
{
    /// @brief A class for handling PID files.
    class PIDFileHandler
    {
    public:
        /// @brief Constructor for the PIDFileHandler class.
        PIDFileHandler();
        ~PIDFileHandler();

        PIDFileHandler(const PIDFileHandler&) = delete;
        PIDFileHandler& operator=(const PIDFileHandler&) = delete;

        PIDFileHandler(PIDFileHandler&&) = default;
        PIDFileHandler& operator=(PIDFileHandler&&) = default;

        /// @brief Reads the PID file and returns its value.
        /// @return The value read from the PID file or 0 if the file does not exist.
        static pid_t ReadPIDFromFile();

    private:
        /// @brief Creates a directory relative to the current path.
        /// @param relativePath  the path of the directory to create.
        /// @return True if the directory was created successfully, false otherwise.
        bool createDirectory(const std::string_view relativePath) const;

        /// @brief Writes the current process' PID to the PID file.
        /// @return True if the PID was written successfully, false otherwise.
        bool writePIDFile() const;

        /// @brief Removes the PID file.
        /// @return True if the file was removed successfully, false otherwise.
        bool removePIDFile() const;

        /// @brief Uses the readlink() function to read the path of the current executable file.
        /// @return A string representing the current process' executable path.
        static std::string GetExecutablePath();
    };

    /// @brief Returns the current status of the daemon.
    /// @return A string representing the daemon's status.
    std::string GetDaemonStatus();

    /// @brief Generates a PID file for the daemon.
    /// @return A PIDFileHandler object that will delete the generated PID file on destruction (RAII style).
    PIDFileHandler GeneratePIDFile();
} // namespace unix_daemon
