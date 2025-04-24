#include <gtest/gtest.h>

#include <os_utils_unix.hpp>

TEST(PidExistsTest, ExistingPid)
{
    EXPECT_TRUE(os_utils::PidExists(getpid()));
}

TEST(GetRunningProcessesTest, FailsOnBadPath)
{
    auto result = os_utils::GetProcessName("/bad/ps", getpid());
    EXPECT_EQ(result, "");
}

TEST(GetRunningProcessesTest, NoProcessesFoundWithNoFilesystemWrapper)
{
    auto result = os_utils::GetRunningProcesses(nullptr);
    EXPECT_TRUE(result.empty());
}
