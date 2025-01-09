#pragma once

#include "column.hpp"

#include <string>
#include <vector>

using TransactionId = unsigned int;

/// @brief Interface for persistence storage.
class Persistence
{
public:
    /// @brief Virtual destructor.
    virtual ~Persistence() = default;

    /// @brief Checks if a specified table exists in the database.
    /// @param tableName The name of the table to check.
    /// @return True if the table exists, false otherwise.
    virtual bool TableExists(const std::string& tableName) = 0;

    /// @brief Creates a new table with specified keys if it doesn't already exist.
    /// @param tableName The name of the table to create.
    /// @param cols Keys specifying the table schema.
    virtual void CreateTable(const std::string& tableName, const Keys& cols) = 0;

    /// @brief Inserts data into a specified table.
    /// @param tableName The name of the table where data is inserted.
    /// @param cols Row with values to insert.
    virtual void Insert(const std::string& tableName, const Row& cols) = 0;

    /// @brief Updates rows in a specified table with optional criteria.
    /// @param tableName The name of the table to update.
    /// @param fields Row with new values to set.
    /// @param selCriteria Optional criteria to filter rows to update.
    /// @param logOp Logical operator to combine selection criteria.
    virtual void Update(const std::string& tableName,
                        const Row& fields,
                        const Criteria& selCriteria = {},
                        LogicalOperator logOp = LogicalOperator::AND) = 0;

    /// @brief Removes rows from a specified table with optional criteria.
    /// @param tableName The name of the table to delete from.
    /// @param selCriteria Optional criteria to filter rows to delete.
    /// @param logOp Logical operator to combine selection criteria.
    virtual void Remove(const std::string& tableName,
                        const Criteria& selCriteria = {},
                        LogicalOperator logOp = LogicalOperator::AND) = 0;

    /// @brief Drops a specified table from the database.
    /// @param tableName The name of the table to drop.
    virtual void DropTable(const std::string& tableName) = 0;

    /// @brief Selects rows from a specified table with optional criteria.
    /// @param tableName The name of the table to select from.
    /// @param fields Names to retrieve.
    /// @param selCriteria Optional selection criteria to filter rows.
    /// @param logOp Logical operator to combine selection criteria (AND/OR).
    /// @param orderBy Names to order the results by.
    /// @param orderType The order type (ASC or DESC).
    /// @param limit The maximum number of rows to retrieve.
    /// @return A vector of rows matching the criteria.
    virtual std::vector<Row> Select(const std::string& tableName,
                                    const Names& fields,
                                    const Criteria& selCriteria = {},
                                    LogicalOperator logOp = LogicalOperator::AND,
                                    const Names& orderBy = {},
                                    OrderType orderType = OrderType::ASC,
                                    unsigned int limit = 0) = 0;

    /// @brief Retrieves the number of rows in a specified table.
    /// @param tableName The name of the table to count rows in.
    /// @param selCriteria Optional selection criteria to filter rows.
    /// @param logOp Logical operator to combine selection criteria (AND/OR).
    /// @return The number of rows in the table.
    virtual int GetCount(const std::string& tableName,
                         const Criteria& selCriteria = {},
                         LogicalOperator logOp = LogicalOperator::AND) = 0;

    /// @brief Retrieves the size in bytes of rows in a specified table.
    /// @param tableName The name of the table to count rows in.
    /// @param fields Names to retrieve.
    /// @return The size in bytes of the rows in the table.
    virtual size_t GetSize(const std::string& tableName, const Names& fields) = 0;

    /// @brief Begins a transaction in the database.
    /// @return The transaction ID.
    virtual TransactionId BeginTransaction() = 0;

    /// @brief Commits a transaction in the database.
    /// @param transactionId The transaction to commit.
    virtual void CommitTransaction(TransactionId transactionId) = 0;

    /// @brief Rolls back a transaction in the database.
    /// @param transactionId The transaction to rollback.
    virtual void RollbackTransaction(TransactionId transactionId) = 0;
};
