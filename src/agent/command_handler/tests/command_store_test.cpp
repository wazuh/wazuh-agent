#include <gtest/gtest.h>

#include <command_store.hpp>
#include <mocks_persistence.hpp>

#include <memory>
#include <optional>
#include <string>
#include <vector>

constexpr char TESTID_5[] = "5";
constexpr char TESTID_9[] = "9";

namespace
{
    // command_store table
    const std::string COMMAND_STORE_TABLE_NAME = "command_store";
    const std::string COMMAND_STORE_ID_COLUMN_NAME = "id";
    const std::string COMMAND_STORE_MODULE_COLUMN_NAME = "module";
    const std::string COMMAND_STORE_COMMAND_COLUMN_NAME = "command";
    const std::string COMMAND_STORE_PARAMETERS_COLUMN_NAME = "parameters";
    const std::string COMMAND_STORE_EXECUTION_MODE_COLUMN_NAME = "execution_mode";
    const std::string COMMAND_STORE_RESULT_COLUMN_NAME = "result";
    const std::string COMMAND_STORE_STATUS_COLUMN_NAME = "status";
    const std::string COMMAND_STORE_TIME_COLUMN_NAME = "time";
} // namespace

class CommandStoreConstructorTest : public ::testing::Test
{
protected:
    void SetUp() override {}
};

TEST_F(CommandStoreConstructorTest, TableExists)
{
    MockPersistence* mockPersistence = nullptr;
    auto mockPersistencePtr = std::make_unique<MockPersistence>();
    mockPersistence = mockPersistencePtr.get();
    EXPECT_CALL(*mockPersistence, TableExists(COMMAND_STORE_TABLE_NAME)).WillOnce(testing::Return(true));

    ASSERT_NO_THROW(std::make_unique<command_store::CommandStore>(".", std::move(mockPersistencePtr)));
}

TEST_F(CommandStoreConstructorTest, TableExistsException)
{
    MockPersistence* mockPersistence = nullptr;
    auto mockPersistencePtr = std::make_unique<MockPersistence>();
    mockPersistence = mockPersistencePtr.get();
    EXPECT_CALL(*mockPersistence, TableExists(COMMAND_STORE_TABLE_NAME))
        .WillOnce(testing::Throw(std::runtime_error("Error TableExists")));

    ASSERT_ANY_THROW(std::make_unique<command_store::CommandStore>(".", std::move(mockPersistencePtr)));
}

TEST_F(CommandStoreConstructorTest, CreateTable)
{
    MockPersistence* mockPersistence = nullptr;
    auto mockPersistencePtr = std::make_unique<MockPersistence>();
    mockPersistence = mockPersistencePtr.get();
    EXPECT_CALL(*mockPersistence, TableExists(COMMAND_STORE_TABLE_NAME)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mockPersistence, CreateTable(COMMAND_STORE_TABLE_NAME, testing::_)).Times(1);

    ASSERT_NO_THROW(std::make_unique<command_store::CommandStore>(".", std::move(mockPersistencePtr)));
}

TEST_F(CommandStoreConstructorTest, CreateTableException)
{
    MockPersistence* mockPersistence = nullptr;
    auto mockPersistencePtr = std::make_unique<MockPersistence>();
    mockPersistence = mockPersistencePtr.get();
    EXPECT_CALL(*mockPersistence, TableExists(COMMAND_STORE_TABLE_NAME)).WillOnce(testing::Return(false));
    EXPECT_CALL(*mockPersistence, CreateTable(COMMAND_STORE_TABLE_NAME, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error CreateTable")));

    ASSERT_ANY_THROW(std::make_unique<command_store::CommandStore>(".", std::move(mockPersistencePtr)));
}

class CommandStoreTest : public ::testing::Test
{
protected:
    std::unique_ptr<command_store::CommandStore> m_commandStore;
    MockPersistence* mockPersistence = nullptr;

    void SetUp() override
    {
        auto mockPersistencePtr = std::make_unique<MockPersistence>();
        mockPersistence = mockPersistencePtr.get();

        EXPECT_CALL(*mockPersistence, TableExists(COMMAND_STORE_TABLE_NAME)).WillOnce(testing::Return(true));

        m_commandStore = std::make_unique<command_store::CommandStore>(".", std::move(mockPersistencePtr));
    }
};

TEST_F(CommandStoreTest, Clear)
{
    EXPECT_CALL(*mockPersistence, Remove(COMMAND_STORE_TABLE_NAME, testing::_, testing::_)).Times(1);

    ASSERT_TRUE(m_commandStore->Clear());
}

TEST_F(CommandStoreTest, ClearException)
{
    EXPECT_CALL(*mockPersistence, Remove(COMMAND_STORE_TABLE_NAME, testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Remove")));

    ASSERT_FALSE(m_commandStore->Clear());
}

TEST_F(CommandStoreTest, GetCount)
{
    const int expectedReturn = 1;

    EXPECT_CALL(*mockPersistence, GetCount(COMMAND_STORE_TABLE_NAME, testing::_, testing::_))
        .WillOnce(testing::Return(expectedReturn));

    ASSERT_EQ(m_commandStore->GetCount(), expectedReturn);
}

TEST_F(CommandStoreTest, GetCountException)
{
    const int expectedReturn = 0;

    EXPECT_CALL(*mockPersistence, GetCount(COMMAND_STORE_TABLE_NAME, testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error GetCount")));

    ASSERT_EQ(m_commandStore->GetCount(), expectedReturn);
}

TEST_F(CommandStoreTest, StoreCommandTrue)
{
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    const module_command::CommandEntry cmd1(TESTID_5,
                                            "Module1",
                                            "{CommandTextHERE}",
                                            {"Parameter1"},
                                            module_command::CommandExecutionMode::ASYNC,
                                            "Result1",
                                            module_command::Status::IN_PROGRESS);

    EXPECT_CALL(*mockPersistence, Insert(COMMAND_STORE_TABLE_NAME, testing::_)).Times(1);

    ASSERT_TRUE(m_commandStore->StoreCommand(cmd1));
}

TEST_F(CommandStoreTest, StoreCommandFalse)
{
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    const module_command::CommandEntry cmd1(TESTID_5,
                                            "Module1",
                                            "{CommandTextHERE}",
                                            {"Parameter1"},
                                            module_command::CommandExecutionMode::ASYNC,
                                            "Result1",
                                            module_command::Status::IN_PROGRESS);

    EXPECT_CALL(*mockPersistence, Insert(COMMAND_STORE_TABLE_NAME, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Insert")));

    ASSERT_FALSE(m_commandStore->StoreCommand(cmd1));
}

TEST_F(CommandStoreTest, StoreCommandTrueCheckFields)
{
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    const module_command::CommandEntry cmd1(TESTID_5,
                                            "Module1",
                                            "{CommandTextHERE}",
                                            {"Parameter1"},
                                            module_command::CommandExecutionMode::ASYNC,
                                            "Result1",
                                            module_command::Status::IN_PROGRESS);

    EXPECT_CALL(
        *mockPersistence,
        Insert(
            testing::Eq(COMMAND_STORE_TABLE_NAME),
            testing::AllOf(
                testing::SizeIs(8),
                testing::Contains(testing::AllOf(
                    testing::Field(&column::ColumnValue::Name, testing::Eq(COMMAND_STORE_ID_COLUMN_NAME)),
                    testing::Field(&column::ColumnValue::Value, testing::Eq(TESTID_5)))),
                testing::Contains(testing::AllOf(
                    testing::Field(&column::ColumnValue::Name, testing::Eq(COMMAND_STORE_MODULE_COLUMN_NAME)),
                    testing::Field(&column::ColumnValue::Value, testing::Eq("Module1")))),
                testing::Contains(testing::AllOf(
                    testing::Field(&column::ColumnValue::Name, testing::Eq(COMMAND_STORE_COMMAND_COLUMN_NAME)),
                    testing::Field(&column::ColumnValue::Value, testing::Eq("{CommandTextHERE}")))),
                testing::Contains(testing::AllOf(
                    testing::Field(&column::ColumnValue::Name, testing::Eq(COMMAND_STORE_TIME_COLUMN_NAME)),
                    testing::Field(&column::ColumnValue::Value, testing::Not(testing::IsEmpty())))),
                testing::Contains(testing::AllOf(
                    testing::Field(&column::ColumnValue::Name, testing::Eq(COMMAND_STORE_PARAMETERS_COLUMN_NAME)),
                    testing::Field(&column::ColumnValue::Value, testing::Eq("[\"Parameter1\"]")))),
                testing::Contains(testing::AllOf(
                    testing::Field(&column::ColumnValue::Name, testing::Eq(COMMAND_STORE_EXECUTION_MODE_COLUMN_NAME)),
                    testing::Field(
                        &column::ColumnValue::Value,
                        testing::Eq(std::to_string(static_cast<int>(module_command::CommandExecutionMode::ASYNC)))))),
                testing::Contains(testing::AllOf(
                    testing::Field(&column::ColumnValue::Name, testing::Eq(COMMAND_STORE_RESULT_COLUMN_NAME)),
                    testing::Field(&column::ColumnValue::Value, testing::Eq("Result1")))),
                testing::Contains(testing::AllOf(
                    testing::Field(&column::ColumnValue::Name, testing::Eq(COMMAND_STORE_STATUS_COLUMN_NAME)),
                    testing::Field(
                        &column::ColumnValue::Value,
                        testing::Eq(std::to_string(static_cast<int>(module_command::Status::IN_PROGRESS)))))))))
        .Times(1);

    ASSERT_TRUE(m_commandStore->StoreCommand(cmd1));
}

TEST_F(CommandStoreTest, DeleteCommandTrue)
{
    EXPECT_CALL(*mockPersistence, Remove(COMMAND_STORE_TABLE_NAME, testing::_, testing::_)).Times(1);

    ASSERT_TRUE(m_commandStore->DeleteCommand(TESTID_5));
}

TEST_F(CommandStoreTest, DeleteCommandFalse)
{
    EXPECT_CALL(*mockPersistence, Remove(COMMAND_STORE_TABLE_NAME, testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Remove")));

    ASSERT_FALSE(m_commandStore->DeleteCommand(TESTID_5));
}

TEST_F(CommandStoreTest, DeleteCommandTrueCheckFilters)
{
    EXPECT_CALL(
        *mockPersistence,
        Remove(COMMAND_STORE_TABLE_NAME,
               testing::AllOf(testing::SizeIs(1),
                              testing::Contains(testing::AllOf(
                                  testing::Field(&column::ColumnValue::Name, testing::Eq(COMMAND_STORE_ID_COLUMN_NAME)),
                                  testing::Field(&column::ColumnValue::Value, testing::Eq(TESTID_5))))),
               testing::_))
        .Times(1);

    ASSERT_TRUE(m_commandStore->DeleteCommand(TESTID_5));
}

TEST_F(CommandStoreTest, GetCommandEmpty)
{
    const std::vector<column::Row> mockRow = {};
    EXPECT_CALL(
        *mockPersistence,
        Select(COMMAND_STORE_TABLE_NAME, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRow));

    auto result = m_commandStore->GetCommand(TESTID_5);

    ASSERT_FALSE(result.has_value());
}

TEST_F(CommandStoreTest, GetCommandReturnCommand)
{
    const std::vector<column::Row> mockRow = {
        {column::ColumnValue(COMMAND_STORE_ID_COLUMN_NAME, column::ColumnType::TEXT, TESTID_5),
         column::ColumnValue(COMMAND_STORE_MODULE_COLUMN_NAME, column::ColumnType::TEXT, "Module1"),
         column::ColumnValue(COMMAND_STORE_COMMAND_COLUMN_NAME, column::ColumnType::TEXT, "{CommandTextHERE}"),
         column::ColumnValue(COMMAND_STORE_PARAMETERS_COLUMN_NAME, column::ColumnType::TEXT, "[\"Parameter1\"]"),
         column::ColumnValue(COMMAND_STORE_EXECUTION_MODE_COLUMN_NAME, column::ColumnType::INTEGER, "1"),
         column::ColumnValue(COMMAND_STORE_RESULT_COLUMN_NAME, column::ColumnType::TEXT, "Result1"),
         column::ColumnValue(COMMAND_STORE_STATUS_COLUMN_NAME, column::ColumnType::INTEGER, "2"),
         column::ColumnValue(COMMAND_STORE_TIME_COLUMN_NAME, column::ColumnType::REAL, "1234567890.0")}};

    EXPECT_CALL(
        *mockPersistence,
        Select(COMMAND_STORE_TABLE_NAME, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRow));

    auto result = m_commandStore->GetCommand(TESTID_5);

    if (!result.has_value())
    {
        FAIL() << "Result is empty";
    }
    const auto& res = *result;
    ASSERT_EQ(res.Id, TESTID_5);
    ASSERT_EQ(res.Module, "Module1");
    ASSERT_EQ(res.Command, "{CommandTextHERE}");
    ASSERT_EQ(res.Parameters.dump(), "[\"Parameter1\"]");
    ASSERT_EQ(res.ExecutionMode, module_command::CommandExecutionMode::ASYNC);
    ASSERT_EQ(res.ExecutionResult.Message, "Result1");
    ASSERT_EQ(res.ExecutionResult.ErrorCode, module_command::Status::IN_PROGRESS);
    ASSERT_EQ(res.Time, 1234567890.0);
}

TEST_F(CommandStoreTest, GetCommandEmptyException)
{
    EXPECT_CALL(
        *mockPersistence,
        Select(COMMAND_STORE_TABLE_NAME, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Select")));

    auto result = m_commandStore->GetCommand(TESTID_5);

    ASSERT_FALSE(result.has_value());
}

TEST_F(CommandStoreTest, GetCommandByStatusEmpty)
{
    const std::vector<column::Row> mockRow = {};
    EXPECT_CALL(
        *mockPersistence,
        Select(COMMAND_STORE_TABLE_NAME, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRow));

    auto result = m_commandStore->GetCommandByStatus(module_command::Status::IN_PROGRESS);

    ASSERT_FALSE(result.has_value());
}

TEST_F(CommandStoreTest, GetCommandByStatusReturnCommand)
{
    const std::vector<column::Row> mockRow = {
        {column::ColumnValue(COMMAND_STORE_ID_COLUMN_NAME, column::ColumnType::TEXT, TESTID_5),
         column::ColumnValue(COMMAND_STORE_MODULE_COLUMN_NAME, column::ColumnType::TEXT, "Module1"),
         column::ColumnValue(COMMAND_STORE_COMMAND_COLUMN_NAME, column::ColumnType::TEXT, "{CommandTextHERE}"),
         column::ColumnValue(COMMAND_STORE_PARAMETERS_COLUMN_NAME, column::ColumnType::TEXT, "[\"Parameter1\"]"),
         column::ColumnValue(COMMAND_STORE_EXECUTION_MODE_COLUMN_NAME, column::ColumnType::INTEGER, "1"),
         column::ColumnValue(COMMAND_STORE_RESULT_COLUMN_NAME, column::ColumnType::TEXT, "Result1"),
         column::ColumnValue(COMMAND_STORE_STATUS_COLUMN_NAME, column::ColumnType::INTEGER, "2"),
         column::ColumnValue(COMMAND_STORE_TIME_COLUMN_NAME, column::ColumnType::REAL, "1234567890.0")}};

    EXPECT_CALL(
        *mockPersistence,
        Select(COMMAND_STORE_TABLE_NAME, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRow));

    auto result = m_commandStore->GetCommandByStatus(module_command::Status::IN_PROGRESS);

    if (!result.has_value())
    {
        FAIL() << "Result is empty";
    }
    const auto& commands = *result;
    ASSERT_EQ(commands.size(), 1);
    ASSERT_EQ(commands[0].Id, TESTID_5);
    ASSERT_EQ(commands[0].Module, "Module1");
    ASSERT_EQ(commands[0].Command, "{CommandTextHERE}");
    ASSERT_EQ(commands[0].Parameters.dump(), "[\"Parameter1\"]");
    ASSERT_EQ(commands[0].ExecutionMode, module_command::CommandExecutionMode::ASYNC);
    ASSERT_EQ(commands[0].ExecutionResult.Message, "Result1");
    ASSERT_EQ(commands[0].ExecutionResult.ErrorCode, module_command::Status::IN_PROGRESS);
    ASSERT_EQ(commands[0].Time, 1234567890.0);
}

TEST_F(CommandStoreTest, GetCommandByStatusReturnCommands2)
{
    const std::vector<column::Row> mockRow = {
        {column::ColumnValue(COMMAND_STORE_ID_COLUMN_NAME, column::ColumnType::TEXT, TESTID_5),
         column::ColumnValue(COMMAND_STORE_MODULE_COLUMN_NAME, column::ColumnType::TEXT, "Module1"),
         column::ColumnValue(COMMAND_STORE_COMMAND_COLUMN_NAME, column::ColumnType::TEXT, "{CommandTextHERE}"),
         column::ColumnValue(COMMAND_STORE_PARAMETERS_COLUMN_NAME, column::ColumnType::TEXT, "[\"Parameter1\"]"),
         column::ColumnValue(COMMAND_STORE_EXECUTION_MODE_COLUMN_NAME, column::ColumnType::INTEGER, "1"),
         column::ColumnValue(COMMAND_STORE_RESULT_COLUMN_NAME, column::ColumnType::TEXT, "Result1"),
         column::ColumnValue(COMMAND_STORE_STATUS_COLUMN_NAME, column::ColumnType::INTEGER, "2"),
         column::ColumnValue(COMMAND_STORE_TIME_COLUMN_NAME, column::ColumnType::REAL, "1234567890.0")},
        {column::ColumnValue(COMMAND_STORE_ID_COLUMN_NAME, column::ColumnType::TEXT, TESTID_9),
         column::ColumnValue(COMMAND_STORE_MODULE_COLUMN_NAME, column::ColumnType::TEXT, "Module2"),
         column::ColumnValue(COMMAND_STORE_COMMAND_COLUMN_NAME, column::ColumnType::TEXT, "{Command2TextHERE}"),
         column::ColumnValue(COMMAND_STORE_PARAMETERS_COLUMN_NAME, column::ColumnType::TEXT, "[\"Parameter2\"]"),
         column::ColumnValue(COMMAND_STORE_EXECUTION_MODE_COLUMN_NAME, column::ColumnType::INTEGER, "1"),
         column::ColumnValue(COMMAND_STORE_RESULT_COLUMN_NAME, column::ColumnType::TEXT, "Result2"),
         column::ColumnValue(COMMAND_STORE_STATUS_COLUMN_NAME, column::ColumnType::INTEGER, "2"),
         column::ColumnValue(COMMAND_STORE_TIME_COLUMN_NAME, column::ColumnType::REAL, "1234567890.0")}};

    EXPECT_CALL(
        *mockPersistence,
        Select(COMMAND_STORE_TABLE_NAME, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRow));

    auto result = m_commandStore->GetCommandByStatus(module_command::Status::IN_PROGRESS);

    if (!result.has_value())
    {
        FAIL() << "Result is empty";
    }
    const auto& commands = *result;
    ASSERT_EQ(commands.size(), 2);
    ASSERT_EQ(commands[0].Id, TESTID_5);
    ASSERT_EQ(commands[0].Module, "Module1");
    ASSERT_EQ(commands[0].Command, "{CommandTextHERE}");
    ASSERT_EQ(commands[0].Parameters.dump(), "[\"Parameter1\"]");
    ASSERT_EQ(commands[0].ExecutionMode, module_command::CommandExecutionMode::ASYNC);
    ASSERT_EQ(commands[0].ExecutionResult.Message, "Result1");
    ASSERT_EQ(commands[0].ExecutionResult.ErrorCode, module_command::Status::IN_PROGRESS);
    ASSERT_EQ(commands[0].Time, 1234567890.0);

    ASSERT_EQ(commands[1].Id, TESTID_9);
    ASSERT_EQ(commands[1].Module, "Module2");
    ASSERT_EQ(commands[1].Command, "{Command2TextHERE}");
    ASSERT_EQ(commands[1].Parameters.dump(), "[\"Parameter2\"]");
    ASSERT_EQ(commands[1].ExecutionMode, module_command::CommandExecutionMode::ASYNC);
    ASSERT_EQ(commands[1].ExecutionResult.Message, "Result2");
    ASSERT_EQ(commands[1].ExecutionResult.ErrorCode, module_command::Status::IN_PROGRESS);
    ASSERT_EQ(commands[1].Time, 1234567890.0);
}

TEST_F(CommandStoreTest, GetCommandByStatusEmptyException)
{
    EXPECT_CALL(
        *mockPersistence,
        Select(COMMAND_STORE_TABLE_NAME, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Select")));

    auto result = m_commandStore->GetCommandByStatus(module_command::Status::IN_PROGRESS);

    ASSERT_FALSE(result.has_value());
}

TEST_F(CommandStoreTest, UpdateCommandTrue)
{
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    const module_command::CommandEntry cmd1(TESTID_5,
                                            "Module1",
                                            "{CommandTextHERE}",
                                            {"Parameter1"},
                                            module_command::CommandExecutionMode::ASYNC,
                                            "Result1",
                                            module_command::Status::IN_PROGRESS);

    EXPECT_CALL(*mockPersistence, Update(COMMAND_STORE_TABLE_NAME, testing::_, testing::_, testing::_)).Times(1);

    ASSERT_TRUE(m_commandStore->UpdateCommand(cmd1));
}

TEST_F(CommandStoreTest, UpdateCommandFalse)
{
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    const module_command::CommandEntry cmd1(TESTID_5,
                                            "Module1",
                                            "{CommandTextHERE}",
                                            {"Parameter1"},
                                            module_command::CommandExecutionMode::ASYNC,
                                            "Result1",
                                            module_command::Status::IN_PROGRESS);

    EXPECT_CALL(*mockPersistence, Update(COMMAND_STORE_TABLE_NAME, testing::_, testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Update")));

    ASSERT_FALSE(m_commandStore->UpdateCommand(cmd1));
}

TEST_F(CommandStoreTest, UpdateCommandTrueCheckFields)
{
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    const module_command::CommandEntry cmd1(TESTID_5,
                                            "Module1",
                                            "{CommandTextHERE}",
                                            {"Parameter1"},
                                            module_command::CommandExecutionMode::ASYNC,
                                            "Result1",
                                            module_command::Status::IN_PROGRESS);

    EXPECT_CALL(
        *mockPersistence,
        Update(testing::Eq(COMMAND_STORE_TABLE_NAME),
               testing::AllOf(
                   testing::SizeIs(5),
                   testing::Contains(testing::AllOf(
                       testing::Field(&column::ColumnValue::Name, testing::Eq(COMMAND_STORE_MODULE_COLUMN_NAME)),
                       testing::Field(&column::ColumnValue::Value, testing::Eq("Module1")))),
                   testing::Contains(testing::AllOf(
                       testing::Field(&column::ColumnValue::Name, testing::Eq(COMMAND_STORE_COMMAND_COLUMN_NAME)),
                       testing::Field(&column::ColumnValue::Value, testing::Eq("{CommandTextHERE}")))),
                   testing::Contains(testing::AllOf(
                       testing::Field(&column::ColumnValue::Name, testing::Eq(COMMAND_STORE_PARAMETERS_COLUMN_NAME)),
                       testing::Field(&column::ColumnValue::Value, testing::Eq("[\"Parameter1\"]")))),
                   testing::Contains(testing::AllOf(
                       testing::Field(&column::ColumnValue::Name, testing::Eq(COMMAND_STORE_RESULT_COLUMN_NAME)),
                       testing::Field(&column::ColumnValue::Value, testing::Eq("Result1")))),
                   testing::Contains(testing::AllOf(
                       testing::Field(&column::ColumnValue::Name, testing::Eq(COMMAND_STORE_STATUS_COLUMN_NAME)),
                       testing::Field(
                           &column::ColumnValue::Value,
                           testing::Eq(std::to_string(static_cast<int>(module_command::Status::IN_PROGRESS))))))),
               testing::AllOf(testing::SizeIs(1),
                              testing::Contains(testing::AllOf(
                                  testing::Field(&column::ColumnValue::Name, testing::Eq(COMMAND_STORE_ID_COLUMN_NAME)),
                                  testing::Field(&column::ColumnValue::Value, testing::Eq(TESTID_5))))),
               testing::_))
        .Times(1);

    ASSERT_TRUE(m_commandStore->UpdateCommand(cmd1));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
