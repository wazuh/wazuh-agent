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

TEST_F(CommandStoreTest, StoreCommandTest)
{
    m_commandStore->Clear();

    command_store::Command cmd1(5, "Module1", "{CommandTextHERE}", "Parameter1", command_store::Status::INPROCESS);
    m_commandStore->StoreCommand(cmd1);
    command_store::Command cmd2(9, "Module2", "{\"Some\"=\"thing\"}", "Parameter2", command_store::Status::INPROCESS);
    m_commandStore->StoreCommand(cmd2);
    command_store::Command cmd3(5, "Module3", "{CommandTextHERE}", "Parameter3", command_store::Status::INPROCESS);
    m_commandStore->StoreCommand(cmd3);
    ASSERT_EQ(m_commandStore->GetCount(), 2);
}

TEST_F(CommandStoreTest, DeleteCommandTest)
{
    int initialCount = m_commandStore->GetCount();

    m_commandStore->DeleteCommand(9);

    ASSERT_EQ(m_commandStore->GetCount(), initialCount - 1);
}

TEST_F(CommandStoreTest, GetCommandTest)
{
    m_commandStore->Clear();

    command_store::Command cmd1(5, "Module1", "{CommandTextHERE}", "Parameter1", command_store::Status::INPROCESS);
    m_commandStore->StoreCommand(cmd1);
    command_store::Command cmd2(9, "Module2", "TestValue9", "Parameter2", command_store::Status::INPROCESS);
    m_commandStore->StoreCommand(cmd2);
    command_store::Command cmd3(11, "Module3", "{CommandTextHERE}", "Parameter3", command_store::Status::INPROCESS);
    m_commandStore->StoreCommand(cmd3);
    ASSERT_EQ(m_commandStore->GetCount(), 3);

    command_store::Command retValue = m_commandStore->GetCommand(9);
    ASSERT_EQ(retValue.m_id, 9);
    ASSERT_EQ(retValue.m_module, "Module2");
    ASSERT_EQ(retValue.m_command, "TestValue9");

    retValue = m_commandStore->GetCommand(11);
    ASSERT_EQ(retValue.m_id, 11);
    ASSERT_EQ(retValue.m_module, "Module3");
    ASSERT_EQ(retValue.m_command, "{CommandTextHERE}");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
