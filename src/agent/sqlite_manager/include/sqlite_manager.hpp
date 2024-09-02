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
        FLOAT
    };

    class Col
    {
    public:
        Col(std::string name,
            const ColumnType type,
            const bool notNull,
            const bool autoIncr,
            const bool primary = false)
            : m_name(std::move(name))
            , m_type(type)
            , m_notNull(notNull)
            , m_autoIncrement(autoIncr)
            , m_primaryKey(primary)
        {
        }

        Col(std::string name, const ColumnType type, std::string value)
            : m_name(std::move(name))
            , m_type(type)
            , m_value(std::move(value))
        {
        }

        std::string m_name;
        ColumnType m_type;
        bool m_notNull;
        bool m_autoIncrement;
        bool m_primaryKey;
        std::string m_value;
    };

    using Row = std::vector<Col>;

    class SQLiteManager
    {
    public:
        SQLiteManager(const std::string& dbName);

        void CreateTable(const std::string& tableName, const std::vector<Col>& cols);
        void Insert(const std::string& tableName, const std::vector<Col>& cols);
        int GetCount(const std::string& tableName);
        std::vector<Row> Select(const std::string& tableName,
                                const std::vector<Col>& fields,
                                const std::vector<Col>& selCriteria = {},
                                LogicalOperator logOp = LogicalOperator::AND);
        void Remove(const std::string& tableName,
                    const std::vector<Col>& selCriteria = {},
                    LogicalOperator logOp = LogicalOperator::AND);
        void Update(const std::string& tableName,
                    const std::vector<Col>& fields,
                    const std::vector<Col>& selCriteria = {},
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
