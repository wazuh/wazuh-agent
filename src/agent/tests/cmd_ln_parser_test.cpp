#include <gtest/gtest.h>

#include <cmd_ln_parser.hpp>

#include <array>
#include <string>

class CommandlineParserTest : public ::testing::Test
{
protected:
    std::string program = "program";
    std::string option = "--option";
    std::string value = "value";
    std::vector<std::string> validOptions = {"--option"};
    std::array<char*, 3> args = {program.data(), option.data(), value.data()};
    CommandlineParser parser {static_cast<int>(args.size()), args.data(), validOptions};
};

TEST_F(CommandlineParserTest, GetOptionValue_OptionExists)
{
    EXPECT_EQ(parser.GetOptionValue(option), value);
}

TEST_F(CommandlineParserTest, GetOptionValue_OptionDoesNotExist)
{
    EXPECT_EQ(parser.GetOptionValue("--nonexistent"), "");
}

TEST_F(CommandlineParserTest, OptionExists_OptionExists)
{
    EXPECT_TRUE(parser.OptionExists(option));
}

TEST_F(CommandlineParserTest, OptionExists_OptionDoesNotExist)
{
    EXPECT_FALSE(parser.OptionExists("--nonexistent"));
}

TEST_F(CommandlineParserTest, GetOptionValue_NoValueForOption)
{
    std::array<char*, 2> noValueArgs = {program.data(), option.data()};
    CommandlineParser noValueParser {static_cast<int>(noValueArgs.size()), noValueArgs.data(), validOptions};

    EXPECT_EQ(noValueParser.GetOptionValue(option), "");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
