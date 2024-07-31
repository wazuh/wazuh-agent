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
    std::string dbName = "testdb-";
    std::string tableName = "test_table";
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

        // Create a new SQLiteStorage instance
        storage = new SQLiteStorage(dbName, tableName);
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
    EXPECT_NO_THROW(storage->Store(message));
    EXPECT_EQ(storage->GetElementCount(), 1);
    EXPECT_NO_THROW(storage->Store(message));
    EXPECT_EQ(storage->GetElementCount(), 2);
}

TEST_F(SQLiteStorageTest, StoreMultipleMessages)
{
    json messages = json::array();
    messages.push_back({{"key", "value1"}});
    messages.push_back({{"key", "value2"}});
    EXPECT_NO_THROW(storage->Store(messages));
    EXPECT_EQ(storage->GetElementCount(), 2);
}

TEST_F(SQLiteStorageTest, RetrieveMessage)
{
    json message = {{"key", "value"}};
    storage->Store(message);
    json retrievedMessage = storage->Retrieve(1);
    EXPECT_EQ(retrievedMessage["key"], "value");
}

TEST_F(SQLiteStorageTest, RetrieveMultipleMessages)
{
    json messages = json::array();
    messages.push_back({{"key", "value1"}});
    messages.push_back({{"key", "value2"}});
    storage->Store(messages);
    json retrievedMessages = storage->RetrieveMultiple(2);
    EXPECT_EQ(retrievedMessages.size(), 2);
}

TEST_F(SQLiteStorageTest, RemoveMessage)
{
    json message = {{"key", "value"}};
    storage->Store(message);
    EXPECT_EQ(storage->Remove(1), 1);
    EXPECT_EQ(storage->GetElementCount(), 0);
}

TEST_F(SQLiteStorageTest, RemoveMultipleMessages)
{
    json messages = json::array();
    messages.push_back({{"key", "value1"}});
    messages.push_back({{"key", "value2"}});
    storage->Store(messages);
    EXPECT_EQ(storage->RemoveMultiple(2), 2);
    EXPECT_EQ(storage->GetElementCount(), 0);
}

TEST_F(SQLiteStorageTest, GetElementCount)
{
    json message = {{"key", "value"}};
    storage->Store(message);
    EXPECT_EQ(storage->GetElementCount(), 1);
}

class SQLiteStorageMultithreadedTest : public ::testing::Test
{
protected:
    std::string dbName = "testdb";
    std::string tableName1 = "test_table1";

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

void storeMessages(SQLiteStorage& storage, const json& messages)
{
    for (const auto& message : messages)
    {
        storage.Store(message);
    }
}

void retrieveMessages(SQLiteStorage& storage, int count, std::vector<json>& retrievedMessages)
{
    retrievedMessages = storage.RetrieveMultiple(count);
}

void removeMessages(SQLiteStorage& storage, int count)
{
    storage.RemoveMultiple(count);
}

TEST_F(SQLiteStorageMultithreadedTest, MultithreadedStoreAndRetrieve)
{
    size_t messagesToStore = 100;
    SQLiteStorage storage1(dbName, tableName1);

    json messages1 = json::array();
    json messages2 = json::array();
    for (size_t i = 0; i < messagesToStore; i++)
    {
        messages1.push_back({{"key" + std::to_string(i), "value" + std::to_string(i)}});
        messages2.push_back({{std::to_string(i) + "key", std::to_string(i) + "value"}});
    }

    // Create threads for storing messages
    std::thread thread1(storeMessages, std::ref(storage1), messages1);
    std::thread thread2(storeMessages, std::ref(storage1), messages2);

    // Join the threads to ensure they complete
    thread1.join();
    thread2.join();

    EXPECT_EQ(storage1.GetElementCount(), 2 * messagesToStore);

    std::thread thread3(removeMessages, std::ref(storage1), messagesToStore);
    std::thread thread4(removeMessages, std::ref(storage1), messagesToStore);

    // Join the threads to ensure they complete
    thread3.join();
    thread4.join();

    EXPECT_EQ(storage1.GetElementCount(), 0);
}
