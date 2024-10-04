#pragma once

#include <string_view>
#include <unistd.h>
#include <vector>

namespace unix_daemon
{
    class PIDFileHandler
    {
    public:
        PIDFileHandler();
        ~PIDFileHandler();

        PIDFileHandler(const PIDFileHandler&) = delete;
        PIDFileHandler& operator=(const PIDFileHandler&) = delete;

        PIDFileHandler(PIDFileHandler&&) = default;
        PIDFileHandler& operator=(PIDFileHandler&&) = default;

        static pid_t ReadPIDFromFile();

    private:
        bool createDirectory(const std::string_view relativePath) const;
        bool writePIDFile() const;
        bool removePIDFile() const;

        static std::string GetExecutablePath();
    };

    std::string GetDaemonStatus();
    PIDFileHandler GeneratePIDFile();
} // namespace unix_daemon
