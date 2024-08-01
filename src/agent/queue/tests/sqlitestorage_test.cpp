#include <filesystem>
#include <iostream>
#include <random>
#include <thread>
#include <vector>

#include "gtest/gtest.h"
#include <nlohmann/json.hpp>

#include "sqlitestorage.h"

using json = nlohmann::json;

class SQLiteStorageTest : public ::testing::Test
{
protected:
    const std::string dbName = "testdb.db";
    const std::string tableName = "test_table";
    const std::vector<std::string> m_vMessageTypeStrings {"test_table", "test_table2"};
    SQLiteStorage* storage;

    void SetUp() override
    {
        // Ensure the database file does not already exist
        std::string filePath = dbName;
        for (const auto& entry : std::filesystem::directory_iterator("."))
        {
            std::string fileFullPath = entry.path();
            size_t found = fileFullPath.find(filePath);
            if (found != std::string::npos)
            {
                std::error_code ec;
                std::filesystem::remove(fileFullPath, ec);
            }
        }

        storage = new SQLiteStorage(dbName, m_vMessageTypeStrings);
    }

    void TearDown() override
    {
        // Clean up: Delete the SQLiteStorage instance and remove the database file
        delete storage;
        std::error_code ec;
        if (std::filesystem::exists(dbName.c_str()))
        {
            std::filesystem::remove(dbName.c_str());
            if (ec)
            {
                std::cerr << "Error removing file: " << ec.message() << std::endl;
            }
        }
    }
};

TEST_F(SQLiteStorageTest, StoreSingleMessage)
{
    json message = {{"key", "value"}};
    EXPECT_NO_THROW(storage->Store(message, tableName));
    EXPECT_EQ(storage->GetElementCount(tableName), 1);
    EXPECT_NO_THROW(storage->Store(message, tableName));
    EXPECT_EQ(storage->GetElementCount(tableName), 2);
}

TEST_F(SQLiteStorageTest, StoreMultipleMessages)
{
    json messages = json::array();
    messages.push_back({{"key", "value1"}});
    messages.push_back({{"key", "value2"}});
    EXPECT_NO_THROW(storage->Store(messages, tableName));
    EXPECT_EQ(storage->GetElementCount(tableName), 2);
}

TEST_F(SQLiteStorageTest, RetrieveMessage)
{
    json message = {{"key", "value"}};
    storage->Store(message, tableName);
    json retrievedMessage = storage->Retrieve(1, tableName);
    EXPECT_EQ(retrievedMessage["key"], "value");
}

TEST_F(SQLiteStorageTest, RetrieveMultipleMessages)
{
    json messages = json::array();
    messages.push_back({{"key", "value1"}});
    messages.push_back({{"key", "value2"}});
    storage->Store(messages, tableName);
    json retrievedMessages = storage->RetrieveMultiple(2, tableName);
    EXPECT_EQ(retrievedMessages.size(), 2);
}

TEST_F(SQLiteStorageTest, RemoveMessage)
{
    json message = {{"key", "value"}};
    storage->Store(message, tableName);
    EXPECT_EQ(storage->Remove(1, tableName), 1);
    EXPECT_EQ(storage->GetElementCount(tableName), 0);
}

TEST_F(SQLiteStorageTest, RemoveMultipleMessages)
{
    json messages = json::array();
    messages.push_back({{"key", "value1"}});
    messages.push_back({{"key", "value2"}});
    storage->Store(messages, tableName);
    EXPECT_EQ(storage->RemoveMultiple(2, tableName), 2);
    EXPECT_EQ(storage->GetElementCount(tableName), 0);
}

TEST_F(SQLiteStorageTest, GetElementCount)
{
    json message = {{"key", "value"}};
    storage->Store(message, tableName);
    EXPECT_EQ(storage->GetElementCount(tableName), 1);
}

class SQLiteStorageMultithreadedTest : public ::testing::Test
{
protected:
    const std::string dbName = "testdb";
    const std::vector<std::string> m_vMessageTypeStrings {"test_table1", "test_table2", "test_table3"};

    void SetUp() override
    {
        // Clean up: Delete the SQLiteStorage instances and remove the database file
        std::error_code ec;
        if (std::filesystem::exists(dbName.c_str()))
        {
            std::filesystem::remove(dbName.c_str());
            if (ec)
            {
                std::cerr << "Error removing file: " << ec.message() << std::endl;
            }
        }
    }

    void TearDown() override {}
};

void storeMessages(SQLiteStorage& storage, const json& messages, const std::string& tableName)
{
    for (const auto& message : messages)
    {
        storage.Store(message, tableName);
    }
}

void retrieveMessages(SQLiteStorage& storage,
                      int count,
                      std::vector<json>& retrievedMessages,
                      const std::string& tableName)
{
    retrievedMessages = storage.RetrieveMultiple(count, tableName);
}

void removeMessages(SQLiteStorage& storage, int count, const std::string& tableName)
{
    storage.RemoveMultiple(count, tableName);
}

TEST_F(SQLiteStorageMultithreadedTest, MultithreadedStoreAndRetrieve)
{
    size_t messagesToStore = 100;

    SQLiteStorage storage1(dbName, m_vMessageTypeStrings);

    json messages1 = json::array();
    json messages2 = json::array();
    for (size_t i = 0; i < messagesToStore; i++)
    {
        messages1.push_back({{"key" + std::to_string(i), "value" + std::to_string(i)}});
        messages2.push_back({{std::to_string(i) + "key", std::to_string(i) + "value"}});
    }

    // Create threads for storing messages
    std::thread thread1(storeMessages, std::ref(storage1), messages1, m_vMessageTypeStrings[0]);
    std::thread thread2(storeMessages, std::ref(storage1), messages2, m_vMessageTypeStrings[0]);

    // Join the threads to ensure they complete
    thread1.join();
    thread2.join();

    EXPECT_EQ(storage1.GetElementCount(m_vMessageTypeStrings[0]), 2 * messagesToStore);

    std::thread thread3(removeMessages, std::ref(storage1), messagesToStore, m_vMessageTypeStrings[0]);
    std::thread thread4(removeMessages, std::ref(storage1), messagesToStore, m_vMessageTypeStrings[0]);

    // Join the threads to ensure they complete
    thread3.join();
    thread4.join();

    EXPECT_EQ(storage1.GetElementCount(m_vMessageTypeStrings[0]), 0);
}
