#include <command_store.hpp>
#include <sqlite_manager.hpp>

#include <gtest/gtest.h>

using namespace command_store;

class CommandStoreTest : public ::testing::Test
{
protected:
    void SetUp() override {}

    void TearDown() override {}
};

TEST(CommandStore, InitialTest)
{
    CommandStore commandStore;

    ASSERT_EQ(1, 1);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
