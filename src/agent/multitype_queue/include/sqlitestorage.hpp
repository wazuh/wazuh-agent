#pragma once

#include <persistence.hpp>

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>
#include <nlohmann/json.hpp>

#include <condition_variable>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

/**
 * @brief SQLite implementation of the Persistence interface.
 *
 * This class provides methods to store, retrieve, and remove JSON messages
 * in a SQLite database.
 */
class SQLiteStorage : public Persistence
{
public:
    SQLiteStorage(const std::string& dbName, const std::vector<std::string>& tableName);

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
     * @param moduleType The type of the module that created the message.
     * @param metadata The metadata message to store.
     * @return The number of stored elements.
     */
    int Store(const nlohmann::json& message,
              const std::string& tableName,
              const std::string& moduleName = "",
              const std::string& moduleType = "",
              const std::string& metadata = "") override;

    /**
     * @brief Retrieve multiple JSON messages.
     *
     * @param n The number of messages to retrieve.
     * @param tableName The name of the table to retrieve the message from.
     * @param moduleName The name of the module that created the message.
     * @param moduleType The module type that created the message.
     * @return A vector of retrieved JSON messages.
     */
    nlohmann::json RetrieveMultiple(int n,
                                    const std::string& tableName,
                                    const std::string& moduleName = "",
                                    const std::string& moduleType = "") override;

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

    /**
     * @brief Get the bytes occupied by elements stored in the specified queue.
     *
     * @param tableName  The name of the table.
     * @return size_t The bytes occupied by elements stored in the specified queue.
     */
    size_t GetElementsStoredSize(const std::string& tableName) override;

    /**
     * @brief Retrieve multiple JSON messages based on size from the specified queue.
     *
     * @param n size occupied by the messages to be retrieved.
     * @param tableName The name of the table to retrieve the message from.
     * @param moduleName The name of the module.
     * @param moduleType The type of the module.
     * @return nlohmann::json The retrieved JSON messages.
     */
    nlohmann::json RetrieveBySize(size_t n,
                                  const std::string& tableName,
                                  const std::string& moduleName = "",
                                  const std::string& moduleType = "") override;

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
    void WaitForDatabaseAccess();

    /**
     * @brief Private method for releasing database access.
     *
     */
    void ReleaseDatabaseAccess();

    /**
     * @brief Intermediate function for processing Retrieve queries.
     */
    nlohmann::json ProcessRequest(SQLite::Statement& sqlStatementQuery, size_t maxSize = 0);

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
