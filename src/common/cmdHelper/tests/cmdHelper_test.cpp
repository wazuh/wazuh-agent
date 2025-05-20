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

// NOLINTBEGIN(bugprone-unchecked-optional-access)

TEST_F(CmdUtilsTest, PipeOpenCmd)
{
    const auto result {Utils::PipeOpen(TEST_CMD)};
    EXPECT_FALSE(result.empty());
}

TEST_F(CmdUtilsTest, ExecCmd)
{
    const auto result {Utils::Exec(TEST_CMD)};
    EXPECT_FALSE(result->StdOut.empty());
    EXPECT_TRUE(result->StdErr.empty());
    EXPECT_EQ(result->ExitCode, 0);
}

TEST_F(CmdUtilsTest, ExecInvalidCmd)
{
    const auto result {Utils::Exec("invalid_command")};
    EXPECT_FALSE(result.has_value());
}

TEST_F(CmdUtilsTest, PipeOpenInvalidCmd)
{
    const auto result {Utils::PipeOpen("invalid_command")};
    EXPECT_TRUE(result.empty());
}

TEST_F(CmdUtilsTest, ExecEmptyCmd)
{
    const auto result {Utils::Exec("")};
    EXPECT_FALSE(result.has_value());
}

TEST_F(CmdUtilsTest, PipeOpenEmptyCmd)
{
    const auto result {Utils::PipeOpen("")};
    EXPECT_TRUE(result.empty());
}

TEST_F(CmdUtilsTest, ExecNestedQuotes)
{
    const auto result = Utils::Exec(TEST_CMD_QUOTES);
    EXPECT_EQ(result->StdOut, TEST_CMD_QUOTES_EXPECTED_RESULT);
    EXPECT_TRUE(result->StdErr.empty());
    EXPECT_EQ(result->ExitCode, 0);
}

TEST_F(CmdUtilsTest, ThrowsOnEmptyString)
{
    EXPECT_THROW(Utils::TokenizeCommand(""), std::runtime_error);
}

TEST_F(CmdUtilsTest, OneWord)
{
    const auto result = Utils::TokenizeCommand("echo");
    ASSERT_EQ(result.size(), 1);
    EXPECT_EQ(result[0], "echo");
}

TEST_F(CmdUtilsTest, MultipleWords)
{
    const auto result = Utils::TokenizeCommand("echo hello world");
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "echo");
    EXPECT_EQ(result[1], "hello");
    EXPECT_EQ(result[2], "world");
}

TEST_F(CmdUtilsTest, QuotedArgument)
{
    const auto result = Utils::TokenizeCommand("echo \"hello world\"");
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "echo");
    EXPECT_EQ(result[1], "hello world");
}

TEST_F(CmdUtilsTest, EscapedCharacters)
{
    const auto result = Utils::TokenizeCommand(R"(echo "escaped test" \\slash)");

    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "echo");
    EXPECT_EQ(result[1], "escaped test");
    EXPECT_EQ(result[2], "\\slash");
}

TEST_F(CmdUtilsTest, SingleQuote)
{
    const auto result = Utils::TokenizeCommand(R"(echo \" >> myFile.txt)");

    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "echo");
    EXPECT_EQ(result[1], "\"");
    EXPECT_EQ(result[2], ">>");
    EXPECT_EQ(result[3], "myFile.txt");
}

TEST_F(CmdUtilsTest, UnclosedQuoteIsHandled)
{
    const auto result = Utils::TokenizeCommand("echo \"hello world");
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "echo");
    EXPECT_EQ(result[1], "hello world");
}

TEST_F(CmdUtilsTest, HandlesExtraSpaces)
{
    const auto result = Utils::TokenizeCommand("echo    hello   world");
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "echo");
    EXPECT_EQ(result[1], "hello");
    EXPECT_EQ(result[2], "world");
}

TEST_F(CmdUtilsTest, NestedEscapedQuotes)
{
    const auto result = Utils::TokenizeCommand(R"(bash -c "echo 'Hello \"nested\" world'")");
    ASSERT_EQ(result.size(), 3);
    EXPECT_EQ(result[0], "bash");
    EXPECT_EQ(result[1], "-c");
    EXPECT_EQ(result[2], "echo 'Hello \"nested\" world'");
}

TEST_F(CmdUtilsTest, EscapedSpaceWithinQuotes)
{
    const auto result = Utils::TokenizeCommand(R"(echo "hello\ world")");
    ASSERT_EQ(result.size(), 2);
    EXPECT_EQ(result[0], "echo");
    EXPECT_EQ(result[1], "hello world");
}

TEST_F(CmdUtilsTest, WindowsRegistryTest)
{
    const auto result =
        Utils::TokenizeCommand(R"(regquery HKLM\Software\Microsoft\Windows\CurrentVersion /v ProductName)");
    ASSERT_EQ(result.size(), 4);
    EXPECT_EQ(result[0], "regquery");
    EXPECT_EQ(result[1], "HKLM\\Software\\Microsoft\\Windows\\CurrentVersion");
    EXPECT_EQ(result[2], "/v");
    EXPECT_EQ(result[3], "ProductName");
}

// NOLINTEND(bugprone-unchecked-optional-access)
