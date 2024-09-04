#include <gtest/gtest.h>

#include <command_store.hpp>

class CommandStoreTest : public ::testing::Test
{
protected:
    std::unique_ptr<command_store::CommandStore> m_commandStore;

    void SetUp() override
    {
        m_commandStore = std::make_unique<command_store::CommandStore>();
    }

    void TearDown() override {}
};

TEST_F(CommandStoreTest, InsertTest)
{

    m_commandStore->Clear();

    command_store::Command cmd1(5, "Module1", "{CommandTextHERE}");
    m_commandStore->StoreCommand(cmd1);
    command_store::Command cmd2(9, "Module2", "{\"Some\"=\"thing\"}");
    m_commandStore->StoreCommand(cmd2);
    command_store::Command cmd3(5, "Module3", "{CommandTextHERE}");
    m_commandStore->StoreCommand(cmd3);
    ASSERT_EQ(m_commandStore->GetCount(), 2);
}

TEST_F(CommandStoreTest, RemoveTest)
{
    int initialCount = m_commandStore->GetCount();

    m_commandStore->DeleteCommand(9);

    ASSERT_EQ(m_commandStore->GetCount(), initialCount - 1);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
