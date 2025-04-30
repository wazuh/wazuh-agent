#include <os_utils.hpp>
#include <os_utils_macos.hpp>

#include <cerrno>
#include <cstdio>
#include <unistd.h>

#include <libproc.h>

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
    if (char pathbuf[PROC_PIDPATHINFO_MAXSIZE]; proc_pidpath(pid, pathbuf, sizeof(pathbuf)) > 0)
    {
        const std::string fullPath(pathbuf);

        if (const auto pos = fullPath.find_last_of('/'); pos != std::string::npos)
        {
            return fullPath.substr(pos + 1);
        }
        return fullPath;
    }
    return {};
}

std::vector<std::string> os_utils::GetRunningProcesses()
{
    std::vector<std::string> processList;

    const pid_t maxPid = 99999;

    for (pid_t pid = 1; pid <= maxPid; ++pid)
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
