#pragma once

#include "column.hpp"
#include "persistence.hpp"

#include <atomic>
#include <map>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace SQLite
{
    class Database;
    class Transaction;
} // namespace SQLite

/// @brief Manages SQLite database operations, including table creation, data insertion, updating, and selection.
class SQLiteManager : public Persistence
{
public:
    /// @brief Initializes a SQLiteManager instance and opens the specified database.
    /// @param dbName The name of the SQLite database file.
    SQLiteManager(const std::string& dbName);

    /// @brief Delete copy constructor.
    SQLiteManager(const SQLiteManager&) = delete;

    /// @brief Delete copy assignment operator.
    SQLiteManager& operator=(const SQLiteManager&) = delete;

    /// @brief Delete move constructor.
    SQLiteManager(SQLiteManager&&) = delete;

    /// @brief Delete move assignment operator.
    SQLiteManager& operator=(SQLiteManager&&) = delete;

    /// @brief Destructor.
    ~SQLiteManager() override;

    /// @brief Checks if a specified table exists in the database.
    /// @param tableName The name of the table to check.
    /// @return True if the table exists, false otherwise.
    bool TableExists(const std::string& tableName) override;

    /// @brief Creates a new table with specified keys if it doesn't already exist.
    /// @param tableName The name of the table to create.
    /// @param cols Keys specifying the table schema.
    void CreateTable(const std::string& tableName, const column::Keys& cols) override;

    /// @brief Inserts data into a specified table.
    /// @param tableName The name of the table where data is inserted.
    /// @param cols Row with values to insert.
    void Insert(const std::string& tableName, const column::Row& cols) override;

    /// @brief Updates rows in a specified table with optional criteria.
    /// @param tableName The name of the table to update.
    /// @param fields Row with new values to set.
    /// @param selCriteria Optional criteria to filter rows to update.
    /// @param logOp Logical operator to combine selection criteria.
    void Update(const std::string& tableName,
                const column::Row& fields,
                const column::Criteria& selCriteria = {},
                column::LogicalOperator logOp = column::LogicalOperator::AND) override;

    /// @brief Removes rows from a specified table with optional criteria.
    /// @param tableName The name of the table to delete from.
    /// @param selCriteria Optional criteria to filter rows to delete.
    /// @param logOp Logical operator to combine selection criteria.
    void Remove(const std::string& tableName,
                const column::Criteria& selCriteria = {},
                column::LogicalOperator logOp = column::LogicalOperator::AND) override;

    /// @brief Drops a specified table from the database.
    /// @param tableName The name of the table to drop.
    void DropTable(const std::string& tableName) override;

    /// @brief Selects rows from a specified table with optional criteria.
    /// @param tableName The name of the table to select from.
    /// @param fields Names to retrieve.
    /// @param selCriteria Optional selection criteria to filter rows.
    /// @param logOp Logical operator to combine selection criteria (AND/OR).
    /// @param orderBy Names to order the results by.
    /// @param orderType The order type (ASC or DESC).
    /// @param limit The maximum number of rows to retrieve.
    /// @return A vector of rows matching the criteria.
    std::vector<column::Row> Select(const std::string& tableName,
                                    const column::Names& fields,
                                    const column::Criteria& selCriteria = {},
                                    column::LogicalOperator logOp = column::LogicalOperator::AND,
                                    const column::Names& orderBy = {},
                                    column::OrderType orderType = column::OrderType::ASC,
                                    int limit = 0) override;

    /// @brief Retrieves the number of rows in a specified table.
    /// @param tableName The name of the table to count rows in.
    /// @param selCriteria Optional selection criteria to filter rows.
    /// @param logOp Logical operator to combine selection criteria (AND/OR).
    /// @return The number of rows in the table.
    int GetCount(const std::string& tableName,
                 const column::Criteria& selCriteria = {},
                 column::LogicalOperator logOp = column::LogicalOperator::AND) override;

    /// @brief Retrieves the size in bytes of rows in a specified table.
    /// @param tableName The name of the table to count rows in.
    /// @param fields Names to retrieve.
    /// @param selCriteria Optional selection criteria to filter rows.
    /// @param logOp Logical operator to combine selection criteria (AND/OR).
    /// @return The size in bytes of the rows in the table.
    size_t GetSize(const std::string& tableName,
                   const column::Names& fields,
                   const column::Criteria& selCriteria = {},
                   column::LogicalOperator logOp = column::LogicalOperator::AND) override;

    /// @brief Begins a transaction in the SQLite database.
    /// @return The transaction ID.
    TransactionId BeginTransaction() override;

    /// @brief Commits a transaction in the SQLite database.
    /// @param transactionId The transaction to commit.
    void CommitTransaction(TransactionId transactionId) override;

    /// @brief Rolls back a transaction in the SQLite database.
    /// @param transactionId The transaction to rollback.
    void RollbackTransaction(TransactionId transactionId) override;

private:
    /// @brief Converts SQLite data types to ColumnType enums.
    /// @param type SQLite data type integer code.
    /// @return Corresponding ColumnType enum.
    column::ColumnType ColumnTypeFromSQLiteType(const int type) const;

    /// @brief Executes a raw SQL query on the database.
    /// @param query The SQL query string to execute.
    void Execute(const std::string& query);

    /// @brief Mutex for thread-safe operations.
    std::mutex m_mutex;

    /// @brief  The name of the SQLite database file.
    const std::string m_dbName;

    /// @brief Pointer to the SQLite database connection.
    std::unique_ptr<SQLite::Database> m_db;

    /// @brief Map of open transactions.
    std::map<TransactionId, std::unique_ptr<SQLite::Transaction>> m_transactions;

    /// @brief The next available transaction ID.
    std::atomic<TransactionId> m_nextTransactionId = 0;
};
