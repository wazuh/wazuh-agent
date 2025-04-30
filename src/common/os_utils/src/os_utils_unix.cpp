#include <os_utils.hpp>
#include <os_utils_unix.hpp>

#include <algorithm>
#include <cerrno>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <unistd.h>

#include <sys/types.h>

std::vector<std::string> os_utils::OsUtils::GetRunningProcesses()
{
    return os_utils::GetRunningProcesses();
}

std::string os_utils::GetProcessName(pid_t pid)
{
    const auto commPath = std::filesystem::path("/proc") / std::to_string(pid) / "comm";

    if (std::ifstream comm(commPath); comm)
    {
        // First line will be the process name
        std::string name;
        std::getline(comm, name);
        return name;
    }

    return {};
}

std::vector<std::string> os_utils::GetRunningProcesses()
{
    std::vector<std::string> processList;

    for (const auto& entry : std::filesystem::directory_iterator("/proc"))
    {
        const auto& filename = entry.path().filename().string();

        if (std::all_of(filename.begin(), filename.end(), ::isdigit))
        {
            const pid_t pid = std::stoi(filename);

            if (auto name = GetProcessName(pid); !name.empty())
            {
                processList.emplace_back(std::move(name));
            }
        }
    }

    return processList;
}
