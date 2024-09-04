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

TEST(CommandStore, InsertlTest)
{
    CommandStore commandStore;

    m_commandStore->Clear();
    command_store::Command cmd1(5, "Module1", "{CommandTextHERE}");
    m_commandStore->StoreCommand(cmd1);
    command_store::Command cmd2(9, "Module2", "{\"Some\"=\"thing\"}");
    m_commandStore->StoreCommand(cmd2);
    command_store::Command cmd3(5, "Module3", "{CommandTextHERE}");
    m_commandStore->StoreCommand(cmd3);
    ASSERT_EQ(m_commandStore->GetCount(), 2);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
