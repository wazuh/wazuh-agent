#pragma once

#include <SQLiteCpp/SQLiteCpp.h>

#include "column.hpp"

#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace sqlite_manager
{
    /// @brief Manages SQLite database operations, including table creation, data insertion, updating, and selection.
    class SQLiteManager
    {
    public:
        /// @brief Initializes a SQLiteManager instance and opens the specified database.
        /// @param dbName The name of the SQLite database file.
        SQLiteManager(const std::string& dbName);

        /// @brief Checks if a specified table exists in the database.
        /// @param tableName The name of the table to check.
        /// @return True if the table exists, false otherwise.
        bool TableExists(const std::string& tableName);

        /// @brief Creates a new table with specified columns if it doesn't already exist.
        /// @param tableName The name of the table to create.
        /// @param cols A vector of columns specifying the table schema.
        void CreateTable(const std::string& tableName, const std::vector<ColumnKey>& cols);

        /// @brief Inserts data into a specified table.
        /// @param tableName The name of the table where data is inserted.
        /// @param cols A vector of columns with values to insert.
        void Insert(const std::string& tableName, const Row& cols);

        /// @brief Updates rows in a specified table with optional criteria.
        /// @param tableName The name of the table to update.
        /// @param fields A vector of columns with new values to set.
        /// @param selCriteria Optional criteria to filter rows to update.
        /// @param logOp Logical operator to combine selection criteria.
        void Update(const std::string& tableName,
                    const Row& fields,
                    const Criteria& selCriteria = {},
                    LogicalOperator logOp = LogicalOperator::AND);

        /// @brief Removes rows from a specified table with optional criteria.
        /// @param tableName The name of the table to delete from.
        /// @param selCriteria Optional criteria to filter rows to delete.
        /// @param logOp Logical operator to combine selection criteria.
        void Remove(const std::string& tableName,
                    const Criteria& selCriteria = {},
                    LogicalOperator logOp = LogicalOperator::AND);

        /// @brief Drops a specified table from the database.
        /// @param tableName The name of the table to drop.
        void DropTable(const std::string& tableName);

        /// @brief Selects rows from a specified table with optional criteria.
        /// @param tableName The name of the table to select from.
        /// @param fields A vector of columns to retrieve.
        /// @param selCriteria Optional selection criteria to filter rows.
        /// @param logOp Logical operator to combine selection criteria (AND/OR).
        /// @param orderBy A vector of columns to order the results by.
        /// @param orderType The order type (ASC or DESC).
        /// @param limit The maximum number of rows to retrieve.
        /// @return A vector of rows matching the criteria.
        std::vector<Row> Select(const std::string& tableName,
                                const std::vector<ColumnName>& fields,
                                const Criteria& selCriteria = {},
                                LogicalOperator logOp = LogicalOperator::AND,
                                const std::vector<ColumnName>& orderBy = {},
                                OrderType orderType = OrderType::ASC,
                                unsigned int limit = 0);

        /// @brief Retrieves the number of rows in a specified table.
        /// @param tableName The name of the table to count rows in.
        /// @param selCriteria Optional selection criteria to filter rows.
        /// @param logOp Logical operator to combine selection criteria (AND/OR).
        /// @return The number of rows in the table.
        int GetCount(const std::string& tableName,
                     const Criteria& selCriteria = {},
                     LogicalOperator logOp = LogicalOperator::AND);

        /// @brief Begins a transaction in the SQLite database.
        /// @return A SQLite transaction object.
        SQLite::Transaction BeginTransaction();

        /// @brief Commits a transaction in the SQLite database.
        /// @param transaction The transaction to commit.
        void CommitTransaction(SQLite::Transaction& transaction);

        /// @brief Rolls back a transaction in the SQLite database.
        /// @param transaction The transaction to roll back.
        void RollbackTransaction(SQLite::Transaction& transaction);

    private:
        /// @brief Converts SQLite data types to ColumnType enums.
        /// @param type SQLite data type integer code.
        /// @return Corresponding ColumnType enum.
        ColumnType ColumnTypeFromSQLiteType(const int type) const;

        /// @brief Executes a raw SQL query on the database.
        /// @param query The SQL query string to execute.
        void Execute(const std::string& query);

        /// @brief Mutex for thread-safe operations.
        std::mutex m_mutex;

        /// @brief  The name of the SQLite database file.
        const std::string m_dbName;

        /// @brief Pointer to the SQLite database connection.
        std::unique_ptr<SQLite::Database> m_db;
    };
} // namespace sqlite_manager
