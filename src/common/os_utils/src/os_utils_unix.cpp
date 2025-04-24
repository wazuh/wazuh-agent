#include <os_utils.hpp>
#include <os_utils_unix.hpp>

#include <fileSmartDeleter.hpp>

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <unistd.h>

namespace
{
    constexpr int MAX_LINE_LENGTH = 2048;

    // TODO: replace with more robust and accurate method
    // to get the maximum PID value
    constexpr pid_t MAX_PID = 32768;
} // namespace

bool os_utils::PidExists(pid_t pid)
{
    return !(getsid(pid) == -1 && errno == ESRCH) && !(getpgid(pid) == -1 && errno == ESRCH);
}

std::string os_utils::GetProcessName(const std::string& ps, int pid)
{
    std::ostringstream cmd;
    cmd << ps << " -p " << pid << " 2> /dev/null";

    const std::unique_ptr<FILE, FileSmartDeleter> pipe(popen(cmd.str().c_str(), "r"));
    if (!pipe)
    {
        return {};
    }

    char buffer[MAX_LINE_LENGTH + 1];

    while (fgets(buffer, sizeof(buffer), pipe.get()))
    {
        const std::string line(buffer);
        const auto colonPosition = line.find(':');
        if (colonPosition == std::string::npos)
        {
            continue;
        }

        const auto spacePosition = line.find(' ', colonPosition);
        if (spacePosition == std::string::npos)
        {
            continue;
        }

        auto trimmed = line.substr(spacePosition + 1);
        trimmed.erase(0, trimmed.find_first_not_of(' '));
        trimmed.erase(trimmed.find_last_not_of('\n') + 1);

        return trimmed;
    }

    return {};
}

std::vector<std::string> os_utils::GetRunningProcesses(std::unique_ptr<IFileSystemWrapper> fs)
{
    if (!fs)
    {
        return {};
    }

    std::string psPath;

    if (fs->is_regular_file("/bin/ps"))
    {
        psPath = "/bin/ps";
    }
    else if (fs->is_regular_file("/usr/bin/ps"))
    {
        psPath = "/usr/bin/ps";
    }
    else
    {
        std::cerr << "'ps' not found.\n";
        return {};
    }

    std::vector<std::string> processList;

    for (pid_t i = 1; i <= MAX_PID; ++i)
    {
        if (PidExists(i))
        {
            if (auto procName = GetProcessName(psPath, i); !procName.empty())
            {
                processList.push_back(std::move(procName));
            }
        }
    }

    return processList;
}
