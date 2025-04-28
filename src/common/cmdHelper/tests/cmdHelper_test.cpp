#include "cmdHelper_test.hpp"
#include "cmdHelper.hpp"

void CmdUtilsTest::SetUp() {};

void CmdUtilsTest::TearDown() {};

#ifdef WIN32
TEST_F(CmdUtilsTest, CmdVersion)
{
    const auto result {Utils::PipeOpen("ver")};
    EXPECT_FALSE(result.empty());
}
#else
TEST_F(CmdUtilsTest, CmdUname)
{
    const auto result {Utils::PipeOpen("uname")};
    EXPECT_FALSE(result.empty());
}
#endif
