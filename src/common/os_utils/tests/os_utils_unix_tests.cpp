#include <gtest/gtest.h>

#include <os_utils_unix.hpp>

TEST(GetRunningProcessesTest, FailsOnBadPid)
{
    const auto result = os_utils::GetProcessName(-1);
    EXPECT_EQ(result, "");
}

TEST(GetRunningProcessesTest, AtLeastOneProcessFound)
{
    const auto result = os_utils::GetRunningProcesses();
    EXPECT_FALSE(result.empty());
}
