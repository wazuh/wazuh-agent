#include <memory>
#include <random>
#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include <nlohmann/json.hpp>

#include <mocks_persistence.hpp>
#include <storage.hpp>

namespace
{
    // column names
    const std::string MODULE_NAME_COLUMN_NAME = "module_name";
    const std::string MODULE_TYPE_COLUMN_NAME = "module_type";
    const std::string METADATA_COLUMN_NAME = "metadata";
    const std::string MESSAGE_COLUMN_NAME = "message";
} // namespace

class StorageConstructorTest : public ::testing::Test
{
protected:
    void SetUp() override {}
};

TEST_F(StorageConstructorTest, TableExists)
{
    const std::vector<std::string> tableName {"test_table.db"};
    MockPersistence* mockPersistence = nullptr;
    auto mockPersistencePtr = std::make_unique<MockPersistence>();
    mockPersistence = mockPersistencePtr.get();
    EXPECT_CALL(*mockPersistence, TableExists("test_table.db")).WillOnce(testing::Return(true));

    ASSERT_NO_THROW(std::make_unique<Storage>(".", tableName, std::move(mockPersistencePtr)));
}

TEST_F(StorageConstructorTest, TableExistsException)
{
    const std::vector<std::string> tableName {"test_table.db"};
    MockPersistence* mockPersistence = nullptr;
    auto mockPersistencePtr = std::make_unique<MockPersistence>();
    mockPersistence = mockPersistencePtr.get();
    EXPECT_CALL(*mockPersistence, TableExists("test_table.db"))
        .WillOnce(testing::Throw(std::runtime_error("Error TableExists")));

    ASSERT_ANY_THROW(std::make_unique<Storage>(".", tableName, std::move(mockPersistencePtr)));
}

TEST_F(StorageConstructorTest, CreateTable)
{
    const std::vector<std::string> tableName {"test_table.db"};
    MockPersistence* mockPersistence = nullptr;
    auto mockPersistencePtr = std::make_unique<MockPersistence>();
    mockPersistence = mockPersistencePtr.get();
    EXPECT_CALL(*mockPersistence, TableExists("test_table.db")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mockPersistence, CreateTable("test_table.db", testing::_)).Times(1);

    ASSERT_NO_THROW(std::make_unique<Storage>(".", tableName, std::move(mockPersistencePtr)));
}

TEST_F(StorageConstructorTest, CreateTableException)
{
    const std::vector<std::string> tableName {"test_table.db"};
    MockPersistence* mockPersistence = nullptr;
    auto mockPersistencePtr = std::make_unique<MockPersistence>();
    mockPersistence = mockPersistencePtr.get();
    EXPECT_CALL(*mockPersistence, TableExists("test_table.db")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mockPersistence, CreateTable("test_table.db", testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error CreateTable")));

    ASSERT_ANY_THROW(std::make_unique<Storage>(".", tableName, std::move(mockPersistencePtr)));
}

TEST_F(StorageConstructorTest, CreateTable2)
{
    const std::vector<std::string> tableName {"test_table.db", "test_table2.db"};
    MockPersistence* mockPersistence = nullptr;
    auto mockPersistencePtr = std::make_unique<MockPersistence>();
    mockPersistence = mockPersistencePtr.get();
    EXPECT_CALL(*mockPersistence, TableExists("test_table.db")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mockPersistence, CreateTable("test_table.db", testing::_)).Times(1);

    EXPECT_CALL(*mockPersistence, TableExists("test_table2.db")).WillOnce(testing::Return(false));
    EXPECT_CALL(*mockPersistence, CreateTable("test_table2.db", testing::_)).Times(1);

    ASSERT_NO_THROW(std::make_unique<Storage>(".", tableName, std::move(mockPersistencePtr)));
}

class StorageTest : public ::testing::Test
{
protected:
    const std::string tableName = "test_table";
    const std::string moduleName = "moduleX";
    const std::vector<std::string> m_vMessageTypeStrings {"test_table", "test_table2"};
    std::unique_ptr<Storage> m_storage;
    MockPersistence* m_mockPersistence = nullptr;

    void SetUp() override
    {
        auto mockPersistencePtr = std::make_unique<MockPersistence>();
        m_mockPersistence = mockPersistencePtr.get();

        EXPECT_CALL(*m_mockPersistence, TableExists("test_table")).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockPersistence, TableExists("test_table2")).WillOnce(testing::Return(true));

        m_storage = std::make_unique<Storage>(".", m_vMessageTypeStrings, std::move(mockPersistencePtr));
    }

    void TearDown() override {}
};

TEST_F(StorageTest, Clear)
{
    const std::vector<std::string> tableNames {tableName};
    EXPECT_CALL(*m_mockPersistence, Remove(tableName, testing::_, testing::_)).Times(1);

    ASSERT_TRUE(m_storage->Clear(tableNames));
}

TEST_F(StorageTest, Clear2)
{
    const std::vector<std::string> tableNames {"test_table.db", "test_table2.db"};

    EXPECT_CALL(*m_mockPersistence, Remove("test_table.db", testing::_, testing::_)).Times(1);
    EXPECT_CALL(*m_mockPersistence, Remove("test_table2.db", testing::_, testing::_)).Times(1);

    ASSERT_TRUE(m_storage->Clear(tableNames));
}

TEST_F(StorageTest, ClearException)
{
    const std::vector<std::string> tableNames {tableName};
    EXPECT_CALL(*m_mockPersistence, Remove(tableName, testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Remove")));

    ASSERT_FALSE(m_storage->Clear(tableNames));
}

TEST_F(StorageTest, StoreSingleMessageArray)
{
    const nlohmann::json message = {{"key", "value"}};

    EXPECT_CALL(*m_mockPersistence, BeginTransaction()).Times(1);
    EXPECT_CALL(*m_mockPersistence, Insert(tableName, testing::_)).Times(1);
    EXPECT_CALL(*m_mockPersistence, CommitTransaction(testing::_)).Times(1);

    EXPECT_EQ(m_storage->Store(message, tableName), 1);
}

TEST_F(StorageTest, StoreSingleMessageNoArray)
{
    const nlohmann::json message = "message-no-array";

    EXPECT_CALL(*m_mockPersistence, BeginTransaction()).Times(1);
    EXPECT_CALL(*m_mockPersistence, Insert(tableName, testing::_)).Times(1);
    EXPECT_CALL(*m_mockPersistence, CommitTransaction(testing::_)).Times(1);

    EXPECT_EQ(m_storage->Store(message, tableName), 1);
}

TEST_F(StorageTest, StoreSingleMessageWithModule)
{
    const nlohmann::json message = {{"key", "value"}};

    EXPECT_CALL(*m_mockPersistence, BeginTransaction()).Times(1);
    EXPECT_CALL(
        *m_mockPersistence,
        Insert(testing::Eq(tableName),
               testing::AllOf(testing::SizeIs(4),
                              testing::Contains(testing::AllOf(
                                  testing::Field(&column::ColumnValue::Name, testing::Eq(MODULE_NAME_COLUMN_NAME)),
                                  testing::Field(&column::ColumnValue::Value, testing::Eq(moduleName)))),
                              testing::Contains(testing::AllOf(
                                  testing::Field(&column::ColumnValue::Name, testing::Eq(MODULE_TYPE_COLUMN_NAME)),
                                  testing::Field(&column::ColumnValue::Value, testing::Eq("")))),
                              testing::Contains(testing::AllOf(
                                  testing::Field(&column::ColumnValue::Name, testing::Eq(METADATA_COLUMN_NAME)),
                                  testing::Field(&column::ColumnValue::Value, testing::Eq("")))),
                              testing::Contains(testing::AllOf(
                                  testing::Field(&column::ColumnValue::Name, testing::Eq(MESSAGE_COLUMN_NAME)),
                                  testing::Field(&column::ColumnValue::Value, testing::Eq("{\"key\":\"value\"}")))))))
        .Times(1);
    EXPECT_CALL(*m_mockPersistence, CommitTransaction(testing::_)).Times(1);

    EXPECT_EQ(m_storage->Store(message, tableName, moduleName), 1);
}

TEST_F(StorageTest, StoreMultipleMessages)
{
    auto messages = nlohmann::json::array();
    messages.push_back({{"key", "value1"}});
    messages.push_back({{"key", "value2"}});

    EXPECT_CALL(*m_mockPersistence, BeginTransaction()).Times(1);
    EXPECT_CALL(*m_mockPersistence, Insert(tableName, testing::_)).Times(2);
    EXPECT_CALL(*m_mockPersistence, CommitTransaction(testing::_)).Times(1);

    EXPECT_EQ(m_storage->Store(messages, tableName), 2);
}

TEST_F(StorageTest, StoreMultipleMessagesFailFirst)
{
    auto messages = nlohmann::json::array();
    messages.push_back({{"key", "value1"}});
    messages.push_back({{"key", "value2"}});

    EXPECT_CALL(*m_mockPersistence, BeginTransaction()).Times(1);

    const testing::Sequence seq;
    EXPECT_CALL(*m_mockPersistence, Insert(tableName, testing::_))
        .InSequence(seq)
        .WillOnce(testing::Throw(std::runtime_error("Error Insert")))
        .WillOnce(testing::Return());
    EXPECT_CALL(*m_mockPersistence, CommitTransaction(testing::_)).Times(1);

    EXPECT_EQ(m_storage->Store(messages, tableName), 1);
}

TEST_F(StorageTest, StoreMultipleMessagesFailSecond)
{
    auto messages = nlohmann::json::array();
    messages.push_back({{"key", "value1"}});
    messages.push_back({{"key", "value2"}});

    EXPECT_CALL(*m_mockPersistence, BeginTransaction()).Times(1);

    const testing::Sequence seq;
    EXPECT_CALL(*m_mockPersistence, Insert(tableName, testing::_))
        .InSequence(seq)
        .WillOnce(testing::Return())
        .WillOnce(testing::Throw(std::runtime_error("Error Insert")));
    EXPECT_CALL(*m_mockPersistence, CommitTransaction(testing::_)).Times(1);

    EXPECT_EQ(m_storage->Store(messages, tableName), 1);
}

TEST_F(StorageTest, StoreMultipleMessagesFailAll)
{
    auto messages = nlohmann::json::array();
    messages.push_back({{"key", "value1"}});
    messages.push_back({{"key", "value2"}});

    EXPECT_CALL(*m_mockPersistence, BeginTransaction()).Times(1);

    const testing::Sequence seq;
    EXPECT_CALL(*m_mockPersistence, Insert(tableName, testing::_))
        .InSequence(seq)
        .WillOnce(testing::Throw(std::runtime_error("Error Insert")))
        .WillOnce(testing::Throw(std::runtime_error("Error Insert")));
    EXPECT_CALL(*m_mockPersistence, CommitTransaction(testing::_)).Times(1);

    EXPECT_EQ(m_storage->Store(messages, tableName), 0);
}

TEST_F(StorageTest, RetrieveMultipleMessages)
{
    const std::vector<column::Row> mockRows = {
        {column::ColumnValue(MODULE_NAME_COLUMN_NAME, column::ColumnType::TEXT, "module1"),
         column::ColumnValue(MODULE_TYPE_COLUMN_NAME, column::ColumnType::TEXT, "type1"),
         column::ColumnValue(METADATA_COLUMN_NAME, column::ColumnType::TEXT, "metadata1"),
         column::ColumnValue(MESSAGE_COLUMN_NAME, column::ColumnType::TEXT, R"({"key":"value1"})")},
        {column::ColumnValue(MODULE_NAME_COLUMN_NAME, column::ColumnType::TEXT, "module2"),
         column::ColumnValue(MODULE_TYPE_COLUMN_NAME, column::ColumnType::TEXT, "type2"),
         column::ColumnValue(METADATA_COLUMN_NAME, column::ColumnType::TEXT, "metadata2"),
         column::ColumnValue(MESSAGE_COLUMN_NAME, column::ColumnType::TEXT, R"({"key":"value2"})")}};

    EXPECT_CALL(*m_mockPersistence,
                Select(tableName, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRows));

    const auto retrievedMessages = m_storage->RetrieveMultiple(2, tableName);
    EXPECT_EQ(retrievedMessages.size(), 2);
}

TEST_F(StorageTest, RetrieveMultipleMessagesLessThanRequested)
{
    const std::vector<column::Row> mockRows = {
        {column::ColumnValue(MODULE_NAME_COLUMN_NAME, column::ColumnType::TEXT, "module1"),
         column::ColumnValue(MODULE_TYPE_COLUMN_NAME, column::ColumnType::TEXT, "type1"),
         column::ColumnValue(METADATA_COLUMN_NAME, column::ColumnType::TEXT, "metadata1"),
         column::ColumnValue(MESSAGE_COLUMN_NAME, column::ColumnType::TEXT, R"({"key":"value1"})")},
        {column::ColumnValue(MODULE_NAME_COLUMN_NAME, column::ColumnType::TEXT, "module2"),
         column::ColumnValue(MODULE_TYPE_COLUMN_NAME, column::ColumnType::TEXT, "type2"),
         column::ColumnValue(METADATA_COLUMN_NAME, column::ColumnType::TEXT, "metadata2"),
         column::ColumnValue(MESSAGE_COLUMN_NAME, column::ColumnType::TEXT, R"({"key":"value2"})")}};

    EXPECT_CALL(*m_mockPersistence,
                Select(tableName, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRows));

    const auto retrievedMessages = m_storage->RetrieveMultiple(3, tableName);
    EXPECT_EQ(retrievedMessages.size(), 2);
}

TEST_F(StorageTest, RetrieveMultipleMessagesWithModule)
{
    const std::vector<column::Row> mockRows = {
        {column::ColumnValue(MODULE_NAME_COLUMN_NAME, column::ColumnType::TEXT, moduleName),
         column::ColumnValue(MODULE_TYPE_COLUMN_NAME, column::ColumnType::TEXT, "type1"),
         column::ColumnValue(METADATA_COLUMN_NAME, column::ColumnType::TEXT, "metadata1"),
         column::ColumnValue(MESSAGE_COLUMN_NAME, column::ColumnType::TEXT, R"({"key":"value1"})")},
        {column::ColumnValue(MODULE_NAME_COLUMN_NAME, column::ColumnType::TEXT, moduleName),
         column::ColumnValue(MODULE_TYPE_COLUMN_NAME, column::ColumnType::TEXT, "type2"),
         column::ColumnValue(METADATA_COLUMN_NAME, column::ColumnType::TEXT, "metadata2"),
         column::ColumnValue(MESSAGE_COLUMN_NAME, column::ColumnType::TEXT, R"({"key":"value2"})")}};

    EXPECT_CALL(
        *m_mockPersistence,
        Select(tableName,
               testing::_,
               testing::AllOf(testing::SizeIs(1),
                              testing::Contains(testing::AllOf(
                                  testing::Field(&column::ColumnValue::Name, testing::Eq(MODULE_NAME_COLUMN_NAME)),
                                  testing::Field(&column::ColumnValue::Value, testing::Eq(moduleName))))),
               testing::_,
               testing::_,
               testing::_,
               testing::_))
        .WillOnce(testing::Return(mockRows));

    const auto retrievedMessages = m_storage->RetrieveMultiple(2, tableName, moduleName);
    EXPECT_EQ(retrievedMessages.size(), 2);
}

TEST_F(StorageTest, RetrieveMultipleSelectFail)
{
    EXPECT_CALL(*m_mockPersistence,
                Select(tableName, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Select")));

    const auto retrievedMessages = m_storage->RetrieveMultiple(2, tableName, moduleName);
    EXPECT_EQ(retrievedMessages.size(), 0);
}

TEST_F(StorageTest, RetrieveBySizeMessage1)
{
    const std::string moduleNameString = moduleName;
    const std::string moduleTypeString = "type1";
    const std::string metadataString = "metadata1";
    const std::string dataString = R"({"key":"value2"})";

    const std::vector<column::Row> mockRows = {
        {column::ColumnValue(MODULE_NAME_COLUMN_NAME, column::ColumnType::TEXT, moduleName),
         column::ColumnValue(MODULE_TYPE_COLUMN_NAME, column::ColumnType::TEXT, moduleTypeString),
         column::ColumnValue(METADATA_COLUMN_NAME, column::ColumnType::TEXT, metadataString),
         column::ColumnValue(MESSAGE_COLUMN_NAME, column::ColumnType::TEXT, dataString)},
        {column::ColumnValue(MODULE_NAME_COLUMN_NAME, column::ColumnType::TEXT, moduleName),
         column::ColumnValue(MODULE_TYPE_COLUMN_NAME, column::ColumnType::TEXT, "type2"),
         column::ColumnValue(METADATA_COLUMN_NAME, column::ColumnType::TEXT, "metadata2"),
         column::ColumnValue(MESSAGE_COLUMN_NAME, column::ColumnType::TEXT, R"({"key":"value2"})")}};

    EXPECT_CALL(*m_mockPersistence,
                Select(tableName, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRows));

    const size_t sizeMessage1 =
        moduleNameString.size() + moduleTypeString.size() + metadataString.size() + dataString.size();

    const auto retrievedMessages = m_storage->RetrieveBySize(sizeMessage1, tableName, moduleName);
    EXPECT_EQ(retrievedMessages.size(), 1);
}

TEST_F(StorageTest, RetrieveBySizeHalfMessage1)
{
    const std::string moduleNameString = moduleName;
    const std::string moduleTypeString = "type1";
    const std::string metadataString = "metadata1";
    const std::string dataString = R"({"key":"value2"})";

    const std::vector<column::Row> mockRows = {
        {column::ColumnValue(MODULE_NAME_COLUMN_NAME, column::ColumnType::TEXT, moduleName),
         column::ColumnValue(MODULE_TYPE_COLUMN_NAME, column::ColumnType::TEXT, moduleTypeString),
         column::ColumnValue(METADATA_COLUMN_NAME, column::ColumnType::TEXT, metadataString),
         column::ColumnValue(MESSAGE_COLUMN_NAME, column::ColumnType::TEXT, dataString)},
        {column::ColumnValue(MODULE_NAME_COLUMN_NAME, column::ColumnType::TEXT, moduleName),
         column::ColumnValue(MODULE_TYPE_COLUMN_NAME, column::ColumnType::TEXT, "type2"),
         column::ColumnValue(METADATA_COLUMN_NAME, column::ColumnType::TEXT, "metadata2"),
         column::ColumnValue(MESSAGE_COLUMN_NAME, column::ColumnType::TEXT, R"({"key":"value2"})")}};

    EXPECT_CALL(*m_mockPersistence,
                Select(tableName, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRows));

    const size_t sizeHalfMessage1 = moduleNameString.size() + moduleTypeString.size();

    const auto retrievedMessages = m_storage->RetrieveBySize(sizeHalfMessage1, tableName, moduleName);
    EXPECT_EQ(retrievedMessages.size(), 1);
}

TEST_F(StorageTest, RetrieveBySizeMessage1HalfMessage2)
{
    const std::string moduleNameString = moduleName;
    const std::string moduleTypeString = "type1";
    const std::string metadataString = "metadata1";
    const std::string dataString = R"({"key":"value2"})";

    const std::vector<column::Row> mockRows = {
        {column::ColumnValue(MODULE_NAME_COLUMN_NAME, column::ColumnType::TEXT, moduleName),
         column::ColumnValue(MODULE_TYPE_COLUMN_NAME, column::ColumnType::TEXT, moduleTypeString),
         column::ColumnValue(METADATA_COLUMN_NAME, column::ColumnType::TEXT, metadataString),
         column::ColumnValue(MESSAGE_COLUMN_NAME, column::ColumnType::TEXT, dataString)},
        {column::ColumnValue(MODULE_NAME_COLUMN_NAME, column::ColumnType::TEXT, moduleName),
         column::ColumnValue(MODULE_TYPE_COLUMN_NAME, column::ColumnType::TEXT, "type2"),
         column::ColumnValue(METADATA_COLUMN_NAME, column::ColumnType::TEXT, "metadata2"),
         column::ColumnValue(MESSAGE_COLUMN_NAME, column::ColumnType::TEXT, R"({"key":"value2"})")}};

    EXPECT_CALL(*m_mockPersistence,
                Select(tableName, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRows));

    const size_t sizeMessage = moduleNameString.size() + moduleTypeString.size() + metadataString.size() +
                               dataString.size() + moduleNameString.size();

    const auto retrievedMessages = m_storage->RetrieveBySize(sizeMessage, tableName, moduleName);
    EXPECT_EQ(retrievedMessages.size(), 2);
}

TEST_F(StorageTest, RetrieveBySizeSelectFail)
{
    EXPECT_CALL(*m_mockPersistence,
                Select(tableName, testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Select")));

    const auto retrievedMessages = m_storage->RetrieveBySize(2, tableName, moduleName);
    EXPECT_EQ(retrievedMessages.size(), 0);
}

TEST_F(StorageTest, GetElementCount)
{
    EXPECT_CALL(*m_mockPersistence, GetCount(tableName, testing::_, testing::_)).WillOnce(testing::Return(1));
    EXPECT_EQ(m_storage->GetElementCount(tableName), 1);
}

TEST_F(StorageTest, GetElementCountGetCountFail)
{
    EXPECT_CALL(*m_mockPersistence, GetCount(tableName, testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error GetCount")));
    EXPECT_EQ(m_storage->GetElementCount(tableName), 0);
}

TEST_F(StorageTest, GetElementsStoredSize)
{
    EXPECT_CALL(*m_mockPersistence, GetSize(tableName, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(1));
    EXPECT_EQ(m_storage->GetElementsStoredSize(tableName), 1);
}

TEST_F(StorageTest, GetElementsStoredSizeGetSizeFail)
{
    EXPECT_CALL(*m_mockPersistence, GetSize(tableName, testing::_, testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error GetSize")));
    EXPECT_EQ(m_storage->GetElementsStoredSize(tableName), 0);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
