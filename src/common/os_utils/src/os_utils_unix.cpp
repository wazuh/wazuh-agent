#include <os_utils.hpp>
#include <os_utils_unix.hpp>

#include <cerrno>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unistd.h>

#if defined(__APPLE__)
#include <libproc.h>
#elif defined(__linux__)
#include <sys/types.h>
#endif

namespace
{
    // TODO: replace with more robust and accurate method
    // to get the maximum PID value
    constexpr pid_t MAX_PID = 32768;
} // namespace

std::vector<std::string> os_utils::OsUtils::GetRunningProcesses()
{
    return os_utils::GetRunningProcesses();
}

bool os_utils::PidExists(pid_t pid)
{
    return !(getsid(pid) == -1 && errno == ESRCH) && !(getpgid(pid) == -1 && errno == ESRCH);
}

std::string os_utils::GetProcessName(pid_t pid)
{
#ifdef __linux__
    const auto commPath = std::filesystem::path("/proc") / std::to_string(pid) / "comm";

    if (std::ifstream comm(commPath); comm)
    {
        // First line will be the process name
        std::string name;
        std::getline(comm, name);
        return name;
    }
#elif defined(__APPLE__)
    if (char pathbuf[PROC_PIDPATHINFO_MAXSIZE]; proc_pidpath(pid, pathbuf, sizeof(pathbuf)) > 0)
    {
        const std::string fullPath(pathbuf);

        if (const auto pos = fullPath.find_last_of('/'); pos != std::string::npos)
        {
            return fullPath.substr(pos + 1);
        }
        return fullPath;
    }
#else
#error "Unsupported platform"
#endif
    return {};
}

std::vector<std::string> os_utils::GetRunningProcesses()
{
    std::vector<std::string> processList;

    for (pid_t pid = 1; pid <= MAX_PID; ++pid)
    {
        if (PidExists(pid))
        {
            if (auto name = GetProcessName(pid); !name.empty())
            {
                processList.emplace_back(std::move(name));
            }
        }
    }

    return processList;
}
