#include <unix_daemon.hpp>

#include <logger.hpp>

#include <cerrno>
#include <filesystem>
#include <fstream>

namespace fs = std::filesystem;

namespace unix_daemon
{
    constexpr std::string_view PID_PATH = "var/run";

    PIDFileHandler::PIDFileHandler()
    {
        writePIDFile();
    }

    PIDFileHandler::~PIDFileHandler()
    {
        removePIDFile();
    }

    bool PIDFileHandler::removePIDFile() const
    {
        const std::string filePath = fmt::format("{}/{}/wazuh-agent.pid", GetExecutablePath(), PID_PATH);
        try
        {
            std::filesystem::remove(filePath);
            return true;
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            LogError("Error removing file: {}", e.what());
            return false;
        }
    }

    bool PIDFileHandler::createDirectory(const std::string_view relativePath) const
    {
        try
        {
            const fs::path curDir = fs::current_path();
            const fs::path fullPath = curDir / relativePath;

            if (fs::exists(fullPath) && fs::is_directory(fullPath))
            {
                return true;
            }

            fs::create_directories(fullPath);
            return true;
        }
        catch (const fs::filesystem_error& e)
        {
            LogCritical("Error creating directory: {}", e.what());
            return false;
        }
    }

    bool PIDFileHandler::writePIDFile() const
    {
        const std::string path = fmt::format("{}/{}", GetExecutablePath(), PID_PATH);
        createDirectory(path);

        const std::string filename = fmt::format("{}/{}/wazuh-agent.pid", GetExecutablePath(), PID_PATH);
        std::ofstream file(filename);

        if (!file.is_open())
        {
            LogCritical("Unable to write PID file: {}. Error: {} ({})", filename.c_str(), errno, std::strerror(errno));
            return false;
        }

        file << getpid();
        file.close();

        return true;
    }

    pid_t PIDFileHandler::ReadPIDFromFile()
    {
        const std::string filename = fmt::format("{}/{}/wazuh-agent.pid", GetExecutablePath(), PID_PATH);
        std::ifstream file(filename);

        if (!file.is_open())
        {
            LogCritical("Error opening file: {}({}) {}", filename.c_str(), errno, std::strerror(errno));
            return 0;
        }

        pid_t value {};
        file >> value;

        if (file.fail())
        {
            LogCritical("Error reading PID file: {}({})", filename.c_str(), errno, std::strerror(errno));
            return 0;
        }

        return value;
    }

    std::string PIDFileHandler::GetExecutablePath()
    {
        std::vector<char> pathBuffer(PATH_MAX);
        const ssize_t len = readlink("/proc/self/exe", pathBuffer.data(), pathBuffer.size() - 1);

        if (len != -1)
        {
            pathBuffer[static_cast<std::size_t>(len)] = '\0';
            const std::filesystem::path exePath(pathBuffer.data());
            return exePath.parent_path().string(); // Return the directory part only
        }
        else
        {
            return "";
        }
    }

    PIDFileHandler GeneratePIDFile()
    {
        return {};
    }

    std::string GetDaemonStatus()
    {
        const pid_t pid = PIDFileHandler::ReadPIDFromFile();
        if (pid == 0)
        {
            return "stopped";
        }
        return "running";
    }
} // namespace unix_daemon
