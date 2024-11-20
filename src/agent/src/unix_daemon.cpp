#include <unix_daemon.hpp>

#include <logger.hpp>

#include <cerrno>
#include <filesystem>
#include <fstream>
#include <sys/file.h>

namespace fs = std::filesystem;

namespace unix_daemon
{
    constexpr std::string_view LOCK_PATH = "var/run";

    LockFileHandler::LockFileHandler()
        : m_lockFileCreated(createLockFile())
    {
    }

    bool LockFileHandler::removeLockFile() const
    {
        const std::string filePath = fmt::format("{}/{}/wazuh-agent.lock", GetExecutablePath(), LOCK_PATH);
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

    bool LockFileHandler::createDirectory(const std::string_view path) const
    {
        try
        {
            const fs::path curDir = fs::current_path();
            const fs::path fullPath = curDir / path;

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

    bool LockFileHandler::createLockFile()
    {
        const std::string path = fmt::format("{}/{}", GetExecutablePath(), LOCK_PATH);
        if (!createDirectory(path))
        {
            LogError("Unable to create lock directory: {}", path);
            return false;
        }

        const std::string filename = fmt::format("{}/wazuh-agent.lock", path);

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-vararg, cppcoreguidelines-avoid-magic-numbers)
        int fd = open(filename.c_str(), O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd == -1)
        {
            LogError("Unable to open lock file: {}. Error: {} ({})", filename.c_str(), errno, std::strerror(errno));
            return false;
        }

        if (flock(fd, LOCK_EX | LOCK_NB) == -1)
        {
            LogDebug("Unable to lock lock file: {}. Error: {} ({})", filename.c_str(), errno, std::strerror(errno));
            close(fd);
            return false;
        }

        LogDebug("Lock file created: {}", filename);

        return true;
    }

    std::string LockFileHandler::GetExecutablePath()
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

    LockFileHandler GenerateLockFile()
    {
        return {};
    }

    std::string GetDaemonStatus()
    {
        LockFileHandler lockFileHandler = GenerateLockFile();

        if (!lockFileHandler.isLockFileCreated())
        {
            return "running";
        }

        lockFileHandler.removeLockFile();
        return "stopped";
    }
} // namespace unix_daemon
