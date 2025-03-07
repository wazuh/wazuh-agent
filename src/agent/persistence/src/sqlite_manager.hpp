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

    /// @copydoc Persistence::TableExists
    bool TableExists(const std::string& tableName) override;

    /// @copydoc Persistence::CreateTable
    void CreateTable(const std::string& tableName, const column::Keys& cols) override;

    /// @copydoc Persistence::Insert
    void Insert(const std::string& tableName, const column::Row& cols) override;

    /// @copydoc Persistence::Update
    void Update(const std::string& tableName,
                const column::Row& fields,
                const column::Criteria& selCriteria = {},
                column::LogicalOperator logOp = column::LogicalOperator::AND) override;

    /// @copydoc Persistence::Remove
    void Remove(const std::string& tableName,
                const column::Criteria& selCriteria = {},
                column::LogicalOperator logOp = column::LogicalOperator::AND) override;

    /// @copydoc Persistence::DropTable
    void DropTable(const std::string& tableName) override;

    /// @copydoc Persistence::Select
    std::vector<column::Row> Select(const std::string& tableName,
                                    const column::Names& fields,
                                    const column::Criteria& selCriteria = {},
                                    column::LogicalOperator logOp = column::LogicalOperator::AND,
                                    const column::Names& orderBy = {},
                                    column::OrderType orderType = column::OrderType::ASC,
                                    int limit = 0) override;

    /// @copydoc Persistence::GetCount
    int GetCount(const std::string& tableName,
                 const column::Criteria& selCriteria = {},
                 column::LogicalOperator logOp = column::LogicalOperator::AND) override;

    /// @copydoc Persistence::GetSize
    size_t GetSize(const std::string& tableName,
                   const column::Names& fields,
                   const column::Criteria& selCriteria = {},
                   column::LogicalOperator logOp = column::LogicalOperator::AND) override;

    /// @copydoc Persistence::BeginTransaction
    TransactionId BeginTransaction() override;

    /// @copydoc Persistence::CommitTransaction
    void CommitTransaction(TransactionId transactionId) override;

    /// @copydoc Persistence::RollbackTransaction
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
