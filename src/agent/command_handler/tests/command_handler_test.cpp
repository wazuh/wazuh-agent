#include <gtest/gtest.h>

#include <command_handler.hpp>

TEST(CommandHandlerTest, CommandHandlerConstructor)
{
    EXPECT_NO_THROW(command_handler::CommandHandler cm("."));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
