#include <os_utils_win.hpp>

#include <os_utils.hpp>

std::vector<std::string> os_utils::OsUtils::GetRunningProcesses()
{
    return os_utils::GetRunningProcesses();
}

std::vector<std::string> os_utils::GetRunningProcesses(const ProcessSnapshotAPI& api)
{
    std::vector<std::string> result;

    HANDLE snapshot = api.CreateSnapshot(TH32CS_SNAPPROCESS, 0);
    if (snapshot == INVALID_HANDLE_VALUE)
    {
        return result;
    }

    PROCESSENTRY32 entry;
    entry.dwSize = sizeof(PROCESSENTRY32);

    if (!api.ProcessFirst(snapshot, &entry))
    {
        CloseHandle(snapshot);
        return result;
    }

    do
    {
        result.push_back({entry.szExeFile});
    } while (api.ProcessNext(snapshot, &entry));

    CloseHandle(snapshot);
    return result;
}
