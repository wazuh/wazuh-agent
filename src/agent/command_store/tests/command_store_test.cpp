#include <command_store.hpp>

#include <gtest/gtest.h>

using namespace command_store;

TEST(CommandStore, InitialTest)
{
    ASSERT_EQ(1, 1);
}


int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
