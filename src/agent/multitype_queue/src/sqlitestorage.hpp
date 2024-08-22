#pragma once

#include <condition_variable>
#include <iostream>
#include <memory>
#include <mutex>
#include <string>
#include <string_view>
#include <sys/stat.h>

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>
#include <stdexcept>

#include "persistence.hpp"

/**
 * @brief SQLite implementation of the Persistence interface.
 *
 * This class provides methods to store, retrieve, and remove JSON messages
 * in a SQLite database.
 */
class SQLiteStorage : public Persistence
{
public:
    SQLiteStorage(const std::string& dbName, const std::vector<std::string> tableName);

    /**
     * @brief Delete copy constructor
     */
    SQLiteStorage(const SQLiteStorage&) = delete;

    /**
     * @brief Delete copy assignment operator
     */
    SQLiteStorage& operator=(const SQLiteStorage&) = delete;

    /**
     * @brief Delete move constructor
     */
    SQLiteStorage(SQLiteStorage&&) = delete;

    /**
     * @brief Delete move assignment operator
     */
    SQLiteStorage& operator=(SQLiteStorage&&) = delete;

    /**
     * @brief Destructor.
     */
    ~SQLiteStorage() override;

    /**
     * @brief Store a JSON message in the storage.
     *
     * @param message The JSON message to store.
     * @param tableName The name of the table to store the message in.
     * @param moduleName The name of the module that created the message.
     * @return The number of stored elements.
     */
    int Store(const json& message, const std::string& tableName, const std::string& moduleName = "") override;

    /**
     * @brief Retrieve a JSON message by its ID.
     *
     * @param id The ID of the message to retrieve.
     * @param tableName The name of the table to retrieve the message from.
     * @param moduleName The name of the module that created the message.
     * @return The retrieved JSON message.
     */
    json Retrieve(int id, const std::string& tableName, const std::string& moduleName = "") override;

    /**
     * @brief Retrieve multiple JSON messages.
     *
     * @param n The number of messages to retrieve.
     * @param tableName The name of the table to retrieve the message from.
     * @param moduleName The name of the module that created the message.
     * @return A vector of retrieved JSON messages.
     */
    json RetrieveMultiple(int n, const std::string& tableName, const std::string& moduleName = "") override;

    /**
     * @brief Remove a JSON message by its ID.
     *
     * @param id The number the message to remove.
     * @param tableName The name of the table to remove the message from.
     * @param moduleName The name of the module that created the message.
     * @return The number of removed elements.
     */
    int Remove(int id, const std::string& tableName, const std::string& moduleName = "") override;

    /**
     * @brief Remove multiple JSON messages.
     *
     * @param n The number of messages to remove.
     * @param tableName The name of the table to remove the message from.
     * @param moduleName The name of the module that created the message.
     * @return The number of removed elements.
     */
    int RemoveMultiple(int n, const std::string& tableName, const std::string& moduleName = "") override;

    /**
     * @brief Get the number of elements in the table.
     * @param tableName The name of the table to retrieve the message from.
     * @param moduleName The name of the module that created the message.
     * @return The number of elements in the table.
     */
    int GetElementCount(const std::string& tableName, const std::string& moduleName = "") override;

private:
    /**
     * @brief Initialize the table in the SQLite database.
     * This method creates the table if it does not already exist.
     * @param tableName The name of the table to initialize.
     */
    void InitializeTable(const std::string& tableName);

    /**
     * @brief Private method for waiting for database access.
     *
     */
    void waitForDatabaseAccess();

    /**
     * @brief Private method for releasing database access.
     *
     */
    void releaseDatabaseAccess();

    /**
     * @brief The name of the SQLite database file.
     */
    const std::string m_dbName;

    /**
     * @brief The name of the table to use for storing messages.
     */
    const std::string m_tableName;

    /**
     * @brief Pointer to the SQLite database connection.
     */
    std::unique_ptr<SQLite::Database> m_db;

    /**
     * @brief Mutex to ensure thread-safe operations.
     */
    std::mutex m_mutex;

    /**
     * @brief condition variable to wait for database access.
     */
    std::condition_variable m_cv;

    /**
     * @brief flag for notifying the use of the db.
     */
    bool m_dbInUse = false;
};
