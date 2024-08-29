#include <gtest/gtest.h>

#include <cmd_ln_parser.hpp>

TEST(CommandlineParserTest, GetOptionValue_OptionExists)
{
    char* argv[] = {(char*)"program", (char*)"--option", (char*)"value"};
    int argc = 3;
    CommandlineParser parser(argc, argv);

    EXPECT_EQ(parser.GetOptionValue("--option"), "value");
}

TEST(CommandlineParserTest, GetOptionValue_OptionDoesNotExist)
{
    char* argv[] = {(char*)"program", (char*)"--option", (char*)"value"};
    int argc = 3;
    CommandlineParser parser(argc, argv);

    EXPECT_EQ(parser.GetOptionValue("--nonexistent"), "");
}

TEST(CommandlineParserTest, OptionExists_OptionExists)
{
    char* argv[] = {(char*)"program", (char*)"--option", (char*)"value"};
    int argc = 3;
    CommandlineParser parser(argc, argv);

    EXPECT_TRUE(parser.OptionExists("--option"));
}

TEST(CommandlineParserTest, OptionExists_OptionDoesNotExist)
{
    char* argv[] = {(char*)"program", (char*)"--option", (char*)"value"};
    int argc = 3;
    CommandlineParser parser(argc, argv);

    EXPECT_FALSE(parser.OptionExists("--nonexistent"));
}

TEST(CommandlineParserTest, GetOptionValue_NoValueForOption)
{
    char* argv[] = {(char*)"program", (char*)"--option"};
    int argc = 2;
    CommandlineParser parser(argc, argv);

    EXPECT_EQ(parser.GetOptionValue("--option"), "");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
