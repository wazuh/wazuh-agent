#include <unix_daemon.hpp>

#include <config.h>
#include <configuration_parser.hpp>
#include <logger.hpp>

#include <cerrno>
#include <filesystem>
#include <fstream>
#include <sys/file.h>

#if defined(__linux__)
#include <unistd.h>
#elif defined(__APPLE__)
#include <mach-o/dyld.h>
#else
#error "Unsupported platform"
#endif

namespace fs = std::filesystem;

namespace unix_daemon
{
    LockFileHandler::LockFileHandler(std::string lockFilePath)
        : m_lockFilePath(std::move(lockFilePath))
        , m_lockFileCreated(createLockFile())
    {
    }

    bool LockFileHandler::removeLockFile() const
    {
        const std::string filePath = fmt::format("{}/wazuh-agent.lock", m_lockFilePath);
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

    bool LockFileHandler::createDirectory(const std::string& path) const
    {
        try
        {
            if (fs::exists(path) && fs::is_directory(path))
            {
                return true;
            }

            fs::create_directories(path);
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
        if (!createDirectory(m_lockFilePath))
        {
            LogError("Unable to create lock directory: {}", m_lockFilePath);
            return false;
        }

        const std::string filename = fmt::format("{}/wazuh-agent.lock", m_lockFilePath);

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

    LockFileHandler GenerateLockFile(const std::string& configFilePath)
    {
        auto configurationParser = configFilePath.empty()
                                       ? configuration::ConfigurationParser()
                                       : configuration::ConfigurationParser(std::filesystem::path(configFilePath));

        const std::string lockFilePath =
            configurationParser.GetConfig<std::string>("agent", "path.run").value_or(config::DEFAULT_RUN_PATH);

        return {LockFileHandler(lockFilePath)};
    }

    std::string GetDaemonStatus(const std::string& configFilePath)
    {
        LockFileHandler lockFileHandler = GenerateLockFile(configFilePath);

        if (!lockFileHandler.isLockFileCreated())
        {
            return "running";
        }

        lockFileHandler.removeLockFile();
        return "stopped";
    }
} // namespace unix_daemon
