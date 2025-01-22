#include <instance_handler.hpp>

#include <config.h>
#include <configuration_parser.hpp>
#include <logger.hpp>

#include <cerrno>
#include <filesystem>
#include <fmt/format.h>
#include <fstream>
#include <sys/file.h>
#include <unistd.h>

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
    void LockFileHandler::removeLockFile() const
    {
        if (!m_lockFileCreated)
            return;

        const std::string filePath = fmt::format("{}/wazuh-agent.lock", m_lockFilePath);
        try
        {
            std::filesystem::remove(filePath);
        }
        catch (const std::filesystem::filesystem_error& e)
        {
            LogError("Error removing file: {}", e.what());
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
            m_errno = errno;
            LogError("Unable to open lock file: {}. Error: {} ({})", filename.c_str(), m_errno, std::strerror(m_errno));
            return false;
        }

        if (flock(fd, LOCK_EX | LOCK_NB) == -1)
        {
            m_errno = errno;
            LogDebug("Unable to lock lock file: {}. Error: {} ({})", filename.c_str(), m_errno, std::strerror(m_errno));
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
            if (lockFileHandler.getErrno() == EAGAIN)
            {
                return "running";
            }
            else
            {
                return fmt::format(
                    "Error: {} ({})", lockFileHandler.getErrno(), std::strerror(lockFileHandler.getErrno()));
            }
        }

        return "stopped";
    }
} // namespace unix_daemon
