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

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    command_store::Command cmd1(5, "Module1", "{CommandTextHERE}", "Parameter1", command_store::Status::IN_PROGRESS);
    m_commandStore->StoreCommand(cmd1);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    command_store::Command cmd2(9, "Module2", R"({"Some"="thing"})", "Parameter2", command_store::Status::IN_PROGRESS);
    m_commandStore->StoreCommand(cmd2);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    command_store::Command cmd3(5, "Module3", "{CommandTextHERE}", "Parameter3", command_store::Status::IN_PROGRESS);
    m_commandStore->StoreCommand(cmd3);
    ASSERT_EQ(m_commandStore->GetCount(), 2);
}

TEST_F(CommandStoreTest, DeleteCommandTest)
{
    int initialCount = m_commandStore->GetCount();

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    m_commandStore->DeleteCommand(9);

    ASSERT_EQ(m_commandStore->GetCount(), initialCount - 1);
}

TEST_F(CommandStoreTest, GetCommandTest)
{
    m_commandStore->Clear();

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    command_store::Command cmd1(5, "Module1", "{CommandTextHERE}", "Parameter1", command_store::Status::IN_PROGRESS);
    m_commandStore->StoreCommand(cmd1);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    command_store::Command cmd2(9, "Module2", "TestValue9", "Parameter2", command_store::Status::IN_PROGRESS);
    m_commandStore->StoreCommand(cmd2);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    command_store::Command cmd3(11, "Module3", "{CommandTextHERE}", "Parameter3", command_store::Status::IN_PROGRESS);
    m_commandStore->StoreCommand(cmd3);
    ASSERT_EQ(m_commandStore->GetCount(), 3);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::optional<command_store::Command> retValue = m_commandStore->GetCommand(9);
    if (retValue.has_value())
    {
        const command_store::Command& cmd = retValue.value();
        ASSERT_EQ(cmd.m_id, 9);
        ASSERT_EQ(cmd.m_module, "Module2");
        ASSERT_EQ(cmd.m_command, "TestValue9");
    }

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    retValue = m_commandStore->GetCommand(11);
    if (retValue.has_value())
    {
        const command_store::Command& cmd = retValue.value();
        ASSERT_EQ(cmd.m_id, 11);
        ASSERT_EQ(cmd.m_module, "Module3");
        ASSERT_EQ(cmd.m_command, "{CommandTextHERE}");
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
