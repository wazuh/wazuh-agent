#ifndef SQLITE_STORAGE_H
#define SQLITE_STORAGE_H

#include <mutex>
#include <string>

#include <SQLiteCpp/SQLiteCpp.h>
#include <SQLiteCpp/VariadicBind.h>
#include <stdexcept>

#include "persistence.h"

/**
 * @brief SQLite implementation of the Persistence interface.
 *
 * This class provides methods to store, retrieve, and remove JSON messages
 * in a SQLite database.
 */
class SQLiteStorage : public Persistence
{
public:
    /**
     * @brief Constructor.
     *
     * @param dbName The name of the SQLite database file.
     * @param tableName The name of the table to use for storing messages.
     */
    SQLiteStorage(const std::string& dbName, const std::string& tableName);

    // Delete copy constructor
    SQLiteStorage(const SQLiteStorage&) = delete;

    // Delete copy assignment operator
    SQLiteStorage& operator=(const SQLiteStorage&) = delete;

    // Delete move constructor
    SQLiteStorage(SQLiteStorage&&) = delete;

    // Delete move assignment operator
    SQLiteStorage& operator=(SQLiteStorage&&) = delete;

    /**
     * @brief Destructor.
     */
    ~SQLiteStorage() override;

    /**
     * @brief Store a JSON message in the storage.
     *
     * @param message The JSON message to store.
     */
    void Store(const json& message) override;

    /**
     * @brief Retrieve a JSON message by its ID.
     *
     * @param id The ID of the message to retrieve.
     * @return The retrieved JSON message.
     */
    json Retrieve(int id) override;

    /**
     * @brief Retrieve multiple JSON messages.
     *
     * @param n The number of messages to retrieve.
     * @return A vector of retrieved JSON messages.
     */
    json RetrieveMultiple(int n) override;

    /**
     * @brief Remove a JSON message by its ID.
     *
     * @param id The ID of the message to remove.
     * @return The number of removed elements.
     */
    int Remove(int id) override;

    /**
     * @brief Remove multiple JSON messages.
     *
     * @param n The number of messages to remove.
     * @return The number of removed elements.
     */
    int RemoveMultiple(int n) override;

    /**
     * @brief Get the number of elements in the table.
     *
     * @return The number of elements in the table.
     */
    int GetElementCount() override;

private:
    /**
     * @brief Initialize the table in the SQLite database.
     *
     * This method creates the table if it does not already exist.
     */
    void InitializeTable();

    /// The name of the SQLite database file.
    std::string m_dbName;

    /// The name of the table to use for storing messages.
    std::string m_tableName;

    /// Pointer to the SQLite database connection.
    //SQLite::Database* m_db;
    std::unique_ptr<SQLite::Database> m_db;

    /// Mutex to ensure thread-safe operations.
    std::mutex m_mtx;
};

#endif // SQLITE_STORAGE_H
