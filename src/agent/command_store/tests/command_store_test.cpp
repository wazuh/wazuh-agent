#include <gtest/gtest.h>

#include <command_store.hpp>
#include <module_command/command_entry.hpp>

#include <memory>
#include <optional>
#include <string>
#include <vector>

constexpr char TESTID_5[] = "5";
constexpr char TESTID_9[] = "9";
constexpr char TESTID_11[] = "11";

class CommandStoreTest : public ::testing::Test
{
protected:
    std::unique_ptr<command_store::CommandStore> m_commandStore;

    void SetUp() override
    {
        m_commandStore = std::make_unique<command_store::CommandStore>(".");
    }

    void TearDown() override {}
};

TEST_F(CommandStoreTest, StoreCommandTest)
{
    m_commandStore->Clear();

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    module_command::CommandEntry cmd1(
        TESTID_5, "Module1", "{CommandTextHERE}", {"Parameter1"}, "Result1", module_command::Status::IN_PROGRESS);
    bool retVal = m_commandStore->StoreCommand(cmd1);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    module_command::CommandEntry cmd2(
        TESTID_9, "Module2", R"({"Some"="thing"})", {"Parameter2"}, "Result2", module_command::Status::IN_PROGRESS);
    retVal = m_commandStore->StoreCommand(cmd2);
    ASSERT_EQ(retVal, true);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    module_command::CommandEntry cmd3(
        TESTID_5, "Module3", "{CommandTextHERE}", {"Parameter3"}, "Result3", module_command::Status::IN_PROGRESS);
    retVal = m_commandStore->StoreCommand(cmd3);
    ASSERT_EQ(retVal, false);

    ASSERT_EQ(m_commandStore->GetCount(), 2);
}

TEST_F(CommandStoreTest, StoreCommandTestParameters)
{
    m_commandStore->Clear();

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    module_command::CommandEntry cmd1(TESTID_5,
                                      "Module1",
                                      "{CommandTextHERE}",
                                      {"Parameter1", "Parameter 2", "3"},
                                      "Result1",
                                      module_command::Status::IN_PROGRESS);
    const bool retVal = m_commandStore->StoreCommand(cmd1);
    ASSERT_TRUE(retVal);

    ASSERT_EQ(m_commandStore->GetCount(), 1);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::optional<module_command::CommandEntry> retValue = m_commandStore->GetCommand(TESTID_5);
    if (retValue.has_value())
    {
        const module_command::CommandEntry& cmd = retValue.value();
        ASSERT_EQ(cmd.Id, TESTID_5);
        ASSERT_EQ(cmd.Module, "Module1");
        ASSERT_EQ(cmd.Command, "{CommandTextHERE}");
        std::vector<std::string> expected = {"Parameter1", "Parameter 2", "3"};
        ASSERT_EQ(cmd.Parameters, expected);
        ASSERT_EQ(cmd.ExecutionResult.Message, "Result1");
        ASSERT_EQ(cmd.ExecutionResult.ErrorCode, module_command::Status::IN_PROGRESS);
    }
    else
    {
        FAIL();
    }
}

TEST_F(CommandStoreTest, UpdateCommandTest)
{
    m_commandStore->Clear();

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    module_command::CommandEntry cmd1(
        TESTID_5, "Module1", "{CommandTextHERE}", {"Parameter1"}, "Result1", module_command::Status::IN_PROGRESS);
    m_commandStore->StoreCommand(cmd1);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    module_command::CommandEntry cmd2(
        TESTID_9, "Module2", R"({"Some"="thing"})", {"Parameter2"}, "Result2", module_command::Status::IN_PROGRESS);
    m_commandStore->StoreCommand(cmd2);

    ASSERT_EQ(m_commandStore->GetCount(), 2);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    module_command::CommandEntry cmdUpdate(TESTID_9,
                                           "Updated Module",
                                           "Updated CommandEntry",
                                           {"Updated Parameter"},
                                           "Updated Result",
                                           module_command::Status::SUCCESS);

    bool retVal = m_commandStore->UpdateCommand(cmdUpdate);
    ASSERT_EQ(retVal, true);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::optional<module_command::CommandEntry> retValue = m_commandStore->GetCommand(TESTID_9);
    if (retValue.has_value())
    {
        const module_command::CommandEntry& cmd = retValue.value();
        ASSERT_EQ(cmd.Id, TESTID_9);
        ASSERT_EQ(cmd.Module, "Updated Module");
        ASSERT_EQ(cmd.Command, "Updated CommandEntry");
        ASSERT_EQ(cmd.Parameters, std::vector<std::string> {"Updated Parameter"});
        ASSERT_EQ(cmd.ExecutionResult.Message, "Updated Result");
        ASSERT_EQ(cmd.ExecutionResult.ErrorCode, module_command::Status::SUCCESS);
    }
    else
    {
        FAIL();
    }

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    module_command::CommandEntry cmdUpdate2(
        TESTID_9, "", "", {}, "Newly Updated Result", module_command::Status::UNKNOWN);

    retVal = m_commandStore->UpdateCommand(cmdUpdate2);
    ASSERT_EQ(retVal, true);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    retValue = m_commandStore->GetCommand(TESTID_9);
    if (retValue.has_value())
    {
        const module_command::CommandEntry& cmd = retValue.value();
        ASSERT_EQ(cmd.Id, TESTID_9);
        ASSERT_EQ(cmd.Module, "Updated Module");
        ASSERT_EQ(cmd.Command, "Updated CommandEntry");
        ASSERT_EQ(cmd.Parameters, std::vector<std::string> {"Updated Parameter"});
        ASSERT_EQ(cmd.ExecutionResult.Message, "Newly Updated Result");
        ASSERT_EQ(cmd.ExecutionResult.ErrorCode, module_command::Status::SUCCESS);
    }
    else
    {
        FAIL();
    }
}

TEST_F(CommandStoreTest, DeleteCommandTest)
{
    int initialCount = m_commandStore->GetCount();

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    m_commandStore->DeleteCommand(TESTID_9);

    ASSERT_EQ(m_commandStore->GetCount(), initialCount - 1);
}

TEST_F(CommandStoreTest, GetCommandTest)
{
    m_commandStore->Clear();

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    module_command::CommandEntry cmd1(
        TESTID_5, "Module1", "{CommandTextHERE}", {"Parameter1"}, "Result1", module_command::Status::IN_PROGRESS);
    m_commandStore->StoreCommand(cmd1);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    module_command::CommandEntry cmd2(
        TESTID_9, "Module2", "TestValue9", {"Parameter2"}, "Result2", module_command::Status::IN_PROGRESS);
    m_commandStore->StoreCommand(cmd2);
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    module_command::CommandEntry cmd3(
        TESTID_11, "Module3", "{CommandTextHERE}", {"Parameter3"}, "Result3", module_command::Status::IN_PROGRESS);
    m_commandStore->StoreCommand(cmd3);
    ASSERT_EQ(m_commandStore->GetCount(), 3);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::optional<module_command::CommandEntry> retValue = m_commandStore->GetCommand(TESTID_9);
    if (retValue.has_value())
    {
        const module_command::CommandEntry& cmd = retValue.value();
        ASSERT_EQ(cmd.Id, TESTID_9);
        ASSERT_EQ(cmd.Module, "Module2");
        ASSERT_EQ(cmd.Command, "TestValue9");
    }

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    retValue = m_commandStore->GetCommand(TESTID_11);
    if (retValue.has_value())
    {
        const module_command::CommandEntry& cmd = retValue.value();
        ASSERT_EQ(cmd.Id, TESTID_11);
        ASSERT_EQ(cmd.Module, "Module3");
        ASSERT_EQ(cmd.Command, "{CommandTextHERE}");
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
