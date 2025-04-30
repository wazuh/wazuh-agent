#include "sca_utils.hpp"
#include <gtest/gtest.h>

using namespace sca;

// NOLINTBEGIN(bugprone-unchecked-optional-access, modernize-raw-string-literal)
TEST(ParseRuleTypeTest, ValidTypes)
{
    auto result = ParseRuleType("f:/path");
    ASSERT_TRUE(result);
    EXPECT_EQ(result->first, WM_SCA_TYPE_FILE);
    EXPECT_EQ(result->second, "/path");

    result = ParseRuleType("r:HKEY_LOCAL_MACHINE\\...");
    ASSERT_TRUE(result);
    EXPECT_EQ(result->first, WM_SCA_TYPE_REGISTRY);

    result = ParseRuleType("p:proc");
    ASSERT_TRUE(result);
    EXPECT_EQ(result->first, WM_SCA_TYPE_PROCESS);

    result = ParseRuleType("d:/dir");
    ASSERT_TRUE(result);
    EXPECT_EQ(result->first, WM_SCA_TYPE_DIR);

    result = ParseRuleType("c:echo");
    ASSERT_TRUE(result);
    EXPECT_EQ(result->first, WM_SCA_TYPE_COMMAND);
}

TEST(ParseRuleTypeTest, NegatedKey)
{
    const auto result = ParseRuleType("!f:/negated");
    ASSERT_TRUE(result);
    EXPECT_EQ(result->first, WM_SCA_TYPE_FILE);
    EXPECT_EQ(result->second, "/negated");
}

TEST(ParseRuleTypeTest, InvalidInputs)
{
    EXPECT_FALSE(ParseRuleType("x:invalid"));
    EXPECT_FALSE(ParseRuleType(":missing"));
    EXPECT_FALSE(ParseRuleType("missingcolon"));
    EXPECT_FALSE(ParseRuleType(""));
}

TEST(GetPatternTest, ValidPattern)
{
    auto result = GetPattern("rule -> pattern");
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "pattern");

    result = GetPattern("x -> y -> z");
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "y -> z");

    result = GetPattern(" -> only");
    ASSERT_TRUE(result);
    EXPECT_EQ(*result, "only");
}

TEST(GetPatternTest, InvalidPattern)
{
    EXPECT_FALSE(GetPattern(""));
    EXPECT_FALSE(GetPattern("no arrow here"));
}

TEST(PatternMatchesTest, SimpleMatch)
{
    EXPECT_TRUE(PatternMatches("match", "match"));
    EXPECT_FALSE(PatternMatches("nope", "match"));
}

TEST(PatternMatchesTest, RegexMatch)
{
    EXPECT_TRUE(PatternMatches("123", "r:\\d+"));
    EXPECT_FALSE(PatternMatches("abc", "r:\\d+"));
}

TEST(PatternMatchesTest, NumericComparison)
{
    EXPECT_TRUE(PatternMatches("123", "n:\\d+ compare == 123"));
    EXPECT_FALSE(PatternMatches("123", "n:\\d+ compare < 100"));
}

TEST(PatternMatchesTest, Negated)
{
    EXPECT_TRUE(PatternMatches("something", "!r:abc"));
    EXPECT_FALSE(PatternMatches("abc", "!r:abc"));
}

TEST(PatternMatchesTest, CompoundPattern)
{
    EXPECT_TRUE(PatternMatches("123abc", "r:\\d+ && r:abc"));
    EXPECT_FALSE(PatternMatches("123abc", "r:\\d+ && r:def"));
}

TEST(PatternMatchesTest, EmptyContent)
{
    EXPECT_FALSE(PatternMatches("", "r:.*"));
}

TEST(PatternMatchesTest, DocExample_LineWithoutCommentWithProtocolAnd2)
{
    const std::string content = "Protocol 2";
    const std::string pattern = "!r:^# && r:Protocol && r:2";
    EXPECT_TRUE(PatternMatches(content, pattern));
}

TEST(PatternMatchesTest, DocExample_CommandOutputStartsWithEnabled)
{
    const std::string content = "enabled";
    const std::string pattern = "r:^enabled";
    EXPECT_TRUE(PatternMatches(content, pattern));
}

TEST(PatternMatchesTest, DocExample_NumericComparisonMaxAuthTries)
{
    const std::string content = "MaxAuthTries	3";
    const std::string pattern = "n:^\\s*MaxAuthTries\\s*\\t*(\\d+) compare <= 4";
    EXPECT_TRUE(PatternMatches(content, pattern));
}

TEST(PatternMatchesTest, DocExample_WholeLineLiteralMatch)
{
    const std::string content = "1";
    const std::string pattern = "1";
    EXPECT_TRUE(PatternMatches(content, pattern));
}

TEST(PatternMatchesTest, DocExample_NegatedRegexMatch)
{
    const std::string content = "maxauthtries 3";
    const std::string pattern = "!r:^\\s*maxauthtries\\s+4\\s*$";
    EXPECT_TRUE(PatternMatches(content, pattern));
}

TEST(PatternMatchesTest, DocExample_UIDCheck)
{
    const std::string content = "user:x:0:0";
    const std::string pattern = "!r:^# && !r:^root: && r:^\\w+:\\w+:0:";
    EXPECT_TRUE(PatternMatches(content, pattern));
}

TEST(PatternMatchesTest, CompoundRule_NegatedCommentAndContainsProtocolAnd2)
{
    const std::string content = "# Some commented line\nProtocol 2\nPort 22";
    const std::string pattern = "!r:^# && r:Protocol && r:2";
    EXPECT_TRUE(PatternMatches(content, pattern));
}

TEST(PatternMatchesTest, NotRegex_ExcludesMatchingLine)
{
    const std::string content = "PasswordAuthentication yes\nPermitRootLogin yes";
    const std::string pattern = "!r:^PasswordAuthentication\\s+no";
    // Should pass: "PasswordAuthentication no" is not present
    EXPECT_TRUE(PatternMatches(content, pattern));
}

TEST(PatternMatchesTest, NotRegex_MatchFailsWhenLineIsPresent)
{
    const std::string content = "PasswordAuthentication no\nPermitRootLogin yes";
    const std::string pattern = "!r:^PasswordAuthentication\\s+no";
    // Should fail: the line is present and matches
    EXPECT_FALSE(PatternMatches(content, pattern));
}

TEST(PatternMatchesTest, NotRegex_WithCompoundMatch)
{
    const std::string content = "# comment\nPermitRootLogin yes\nPasswordAuthentication yes";
    const std::string pattern = "!r:^# && r:PermitRootLogin && r:yes";
    // Should pass: "PermitRootLogin yes" line does not start with # and contains the other tokens
    EXPECT_TRUE(PatternMatches(content, pattern));
}

TEST(PatternMatchesTest, NotRegex_WithCompoundFailing)
{
    const std::string content = "# PermitRootLogin yes";
    const std::string pattern = "!r:^# && r:PermitRootLogin && r:yes";
    // Should fail: the only line containing "PermitRootLogin" also starts with #
    EXPECT_FALSE(PatternMatches(content, pattern));
}

// NOLINTEND(bugprone-unchecked-optional-access, modernize-raw-string-literal)
