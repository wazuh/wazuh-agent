#pragma once

#include <SQLiteCpp/SQLiteCpp.h>

#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace sqlite_manager
{
    enum class LogicalOperator
    {
        AND,
        OR
    };

    enum class ColumnType
    {
        INTEGER,
        TEXT,
        REAL
    };

    class Column
    {
    public:
        Column(std::string name,
               const ColumnType type,
               const bool notNull,
               const bool autoIncr,
               const bool primary = false)
            : Name(std::move(name))
            , Type(type)
            , NotNull(notNull)
            , AutoIncrement(autoIncr)
            , PrimaryKey(primary)
        {
        }

        Column(std::string name, const ColumnType type, std::string value)
            : Name(std::move(name))
            , Type(type)
            , Value(std::move(value))
        {
        }

        std::string Name;
        ColumnType Type;
        bool NotNull;
        bool AutoIncrement;
        bool PrimaryKey;
        std::string Value;
    };

    using Row = std::vector<Column>;

    class SQLiteManager
    {
    public:
        SQLiteManager(const std::string& dbName);

        void CreateTable(const std::string& tableName, const std::vector<Column>& cols);
        bool TableExists(const std::string& tableName) const;
        void Insert(const std::string& tableName, const std::vector<Column>& cols);
        int GetCount(const std::string& tableName);
        std::vector<Row> Select(const std::string& tableName,
                                const std::vector<Column>& fields,
                                const std::vector<Column>& selCriteria = {},
                                LogicalOperator logOp = LogicalOperator::AND);
        void Remove(const std::string& tableName,
                    const std::vector<Column>& selCriteria = {},
                    LogicalOperator logOp = LogicalOperator::AND);
        void Update(const std::string& tableName,
                    const std::vector<Column>& fields,
                    const std::vector<Column>& selCriteria = {},
                    LogicalOperator logOp = LogicalOperator::AND);
        void DropTable(const std::string& tableName);
        SQLite::Transaction BeginTransaction();
        void CommitTransaction(SQLite::Transaction& transaction);
        void RollbackTransaction(SQLite::Transaction& transaction);

    private:
        /**
         * @brief Mutex to ensure thread-safe operations.
         */
        std::mutex m_mutex;

        /**
         * @brief The name of the SQLite database file.
         */
        const std::string m_dbName;

        /**
         * @brief Pointer to the SQLite database connection.
         */
        std::unique_ptr<SQLite::Database> m_db;

        ColumnType ColumnTypeFromSQLiteType(const int type) const;
        void Execute(const std::string& query);
    };
} // namespace sqlite_manager
