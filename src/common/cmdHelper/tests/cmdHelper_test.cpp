#include "cmdHelper_test.hpp"
#include "cmdHelper.hpp"

#include <string>

#ifdef WIN32
const std::string TEST_CMD = "whoami";
const std::string TEST_CMD_QUOTES = "powershell -Command \" Write-Output 'Hello world'\"";
const std::string TEST_CMD_QUOTES_EXPECTED_RESULT = "Hello world\r\n";
#else
const std::string TEST_CMD = "uname";
const std::string TEST_CMD_QUOTES = "bash -c \" echo 'Hello world'\"";
const std::string TEST_CMD_QUOTES_EXPECTED_RESULT = "Hello world\n";
#endif

TEST_F(CmdUtilsTest, PipeOpenCmd)
{
    const auto result {Utils::PipeOpen(TEST_CMD)};
    EXPECT_FALSE(result.empty());
}

TEST_F(CmdUtilsTest, ExecCmd)
{
    const auto result {Utils::Exec(TEST_CMD)};
    EXPECT_FALSE(result.StdOut.empty());
    EXPECT_TRUE(result.StdErr.empty());
    EXPECT_EQ(result.ExitCode, 0);
}

TEST_F(CmdUtilsTest, ExecInvalidCmd)
{
    const auto result {Utils::Exec("invalid_command")};
    EXPECT_TRUE(result.StdOut.empty());
    EXPECT_FALSE(result.StdErr.empty());
    EXPECT_NE(result.ExitCode, 0);
}

TEST_F(CmdUtilsTest, PipeOpenInvalidCmd)
{
    const auto result {Utils::PipeOpen("invalid_command")};
    EXPECT_TRUE(result.empty());
}

TEST_F(CmdUtilsTest, ExecEmptyCmd)
{
    const auto result {Utils::Exec("")};
    EXPECT_TRUE(result.StdOut.empty());
    EXPECT_FALSE(result.StdErr.empty());
    EXPECT_NE(result.ExitCode, 0);
}

TEST_F(CmdUtilsTest, PipeOpenEmptyCmd)
{
    const auto result {Utils::PipeOpen("")};
    EXPECT_TRUE(result.empty());
}

TEST_F(CmdUtilsTest, ExecNestedQuotes)
{
    const auto result = Utils::Exec(TEST_CMD_QUOTES);
    EXPECT_EQ(result.StdOut, TEST_CMD_QUOTES_EXPECTED_RESULT);
    EXPECT_TRUE(result.StdErr.empty());
    EXPECT_EQ(result.ExitCode, 0);
}
