#include <gtest/gtest.h>

#include <command_manager.hpp>

TEST(CommandManagerTest, CommandManagerConstructor)
{
    EXPECT_NO_THROW(command_manager::CommandManager cm());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}