#include "linuxInfoHelper_test.hpp"
#include "cmdHelper.hpp"
#include "linuxInfoHelper.hpp"

void LinuxInfoHelperTest::SetUp() {};

void LinuxInfoHelperTest::TearDown() {};

TEST_F(LinuxInfoHelperTest, getBootTime)
{
    const auto btimeNumFromFunc {Utils::getBootTime()};

    const auto btimeStr {Utils::Exec("grep 'btime' /proc/stat | awk '{print $2}'")};
    const auto btimeNumFromProc {std::stoull(btimeStr)};

    EXPECT_FALSE(btimeNumFromFunc != btimeNumFromProc);
}

TEST_F(LinuxInfoHelperTest, timeTick2unixTime)
{
    const auto startTimeStr {Utils::Exec("awk '{print $22}' /proc/1/stat")};
    const auto startTimeNum {std::stoul(startTimeStr)};
    const auto startTimeUnixFunc {Utils::timeTick2unixTime(startTimeNum)};

    const auto startTimeUnixCmd {std::stoul(Utils::Exec("date -d \"`ps -o lstart -p 1|tail -n 1`\" +%s"))};

    EXPECT_FALSE(startTimeUnixFunc != startTimeUnixCmd);
}
