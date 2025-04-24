#include <gtest/gtest.h>

#include <os_utils_win.hpp>

TEST(GetWindowsProcessListTest, MockedProcessList)
{
    const std::vector<std::string> fakeNames = {"procA.exe", "procB.exe"};

    std::vector<PROCESSENTRY32> entries;
    for (const auto& name : fakeNames)
    {
        PROCESSENTRY32 entry {};
        entry.dwSize = sizeof(entry);
        strcpy_s(entry.szExeFile, name.c_str());
        entries.push_back(entry);
    }

    size_t callIndex = 0;

    ProcessSnapshotAPI mockAPI;
    mockAPI.CreateSnapshot = [](DWORD, DWORD) -> HANDLE
    {
        return reinterpret_cast<HANDLE>(1);
    };
    mockAPI.ProcessFirst = [&](HANDLE, LPPROCESSENTRY32 out) -> BOOL
    {
        if (entries.empty())
        {
            return FALSE;
        }
        *out = entries[0];
        callIndex = 1;
        return TRUE;
    };
    mockAPI.ProcessNext = [&](HANDLE, LPPROCESSENTRY32 out) -> BOOL
    {
        if (callIndex >= entries.size())
        {
            return FALSE;
        }
        *out = entries[callIndex++];
        return TRUE;
    };

    const auto list = os_utils::GetRunningProcesses(mockAPI);
    ASSERT_EQ(list.size(), 2);
    EXPECT_EQ(list[0], "procA.exe");
    EXPECT_EQ(list[1], "procB.exe");
}
