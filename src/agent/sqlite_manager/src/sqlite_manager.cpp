#include <sqlite_manager.hpp>

#include <logger.hpp>

#include <fmt/format.h>
#include <map>

namespace sqlite_manager
{
    const std::map<ColumnType, std::string> MAP_COL_TYPE_STRING {
        {ColumnType::INTEGER, "INTEGER"}, {ColumnType::TEXT, "TEXT"}, {ColumnType::REAL, "REAL"}};
    const std::map<LogicalOperator, std::string> MAP_LOGOP_STRING {{LogicalOperator::AND, "AND"},
                                                                   {LogicalOperator::OR, "OR"}};

    ColumnType SQLiteManager::ColumnTypeFromSQLiteType(const int type) const
    {
        if (type == SQLite::INTEGER)
            return ColumnType::INTEGER;

        if (type == SQLite::FLOAT)
            return ColumnType::REAL;

        return ColumnType::TEXT;
    }

    SQLiteManager::SQLiteManager(const std::string& dbName)
        : m_dbName(dbName)
        , m_db(std::make_unique<SQLite::Database>(dbName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE))
    {
        m_db->exec("PRAGMA journal_mode=WAL;");
    }

    void SQLiteManager::CreateTable(const std::string& tableName, const std::vector<Column>& cols)
    {
        std::vector<std::string> pk;
        std::vector<std::string> fields;
        for (const Column& col : cols)
        {
            std::string field = fmt::format(
                "{} {}{}", col.m_name, MAP_COL_TYPE_STRING.at(col.m_type), (col.m_notNull) ? " NOT NULL" : "");
            if (col.m_primaryKey)
            {
                pk.push_back(col.m_autoIncrement ? fmt::format("{} AUTOINCREMENT", col.m_name) : col.m_name);
            }
            fields.push_back(field);
        }

        std::string primaryKey = pk.empty() ? "" : fmt::format(", PRIMARY KEY({})", fmt::join(pk, ", "));
        std::string queryString = fmt::format(
            "CREATE TABLE IF NOT EXISTS {} ({}{})", tableName, fmt::format("{}", fmt::join(fields, ", ")), primaryKey);

        Execute(queryString);
    }

    void SQLiteManager::Insert(const std::string& tableName, const std::vector<Column>& cols)
    {
        std::vector<std::string> names;
        std::vector<std::string> values;

        for (const Column& col : cols)
        {
            names.push_back(col.m_name);
            if (col.m_type == ColumnType::TEXT)
            {
                values.push_back(fmt::format("'{}'", col.m_value));
            }
            else
                values.push_back(col.m_value);
        }

        std::string queryString =
            fmt::format("INSERT INTO {} ({}) VALUES ({})", tableName, fmt::join(names, ", "), fmt::join(values, ", "));

        Execute(queryString);
    }

    int SQLiteManager::GetCount(const std::string& tableName)
    {
        std::string queryString = fmt::format("SELECT COUNT(*) FROM {}", tableName);

        try
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            SQLite::Statement query(*m_db, queryString);
            int count = 0;

            if (query.executeStep())
            {
                count = query.getColumn(0).getInt();
            }
            else
            {
                LogError("Error getting element count.");
            }
            return count;
        }
        catch (const std::exception& e)
        {
            LogError("Error during GetCount operation: {}.", e.what());
            throw;
        }
    }

    std::vector<Row> SQLiteManager::Select(const std::string& tableName,
                                           const std::vector<Column>& fields,
                                           const std::vector<Column>& selCriteria,
                                           LogicalOperator logOp)
    {
        std::string selectedFields;
        if (fields.empty())
        {
            selectedFields = "*";
        }
        else
        {
            std::vector<std::string> fieldNames;
            fieldNames.reserve(fields.size());

            for (auto& col : fields)
            {
                fieldNames.push_back(col.m_name);
            }
            selectedFields = fmt::format("{}", fmt::join(fieldNames, ", "));
        }

        std::string condition;
        if (!selCriteria.empty())
        {
            std::vector<std::string> conditions;
            for (auto& col : selCriteria)
            {
                if (col.m_type == ColumnType::TEXT)
                    conditions.push_back(fmt::format("{} = '{}'", col.m_name, col.m_value));
                else
                    conditions.push_back(fmt::format("{} = {}", col.m_name, col.m_value));
            }
            condition = fmt::format("WHERE {}", fmt::join(conditions, fmt::format(" {} ", MAP_LOGOP_STRING.at(logOp))));
        }

        std::string queryString = fmt::format("SELECT {} FROM {} {}", selectedFields, tableName, condition);

        std::vector<Row> results;
        try
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            SQLite::Statement query(*m_db, queryString);
            while (query.executeStep())
            {
                int nColumns = query.getColumnCount();
                std::vector<Column> queryFields;
                queryFields.reserve(static_cast<size_t>(nColumns));
                for (int i = 0; i < nColumns; i++)
                {
                    sqlite_manager::Column field(query.getColumn(i).getName(),
                                                 ColumnTypeFromSQLiteType(query.getColumn(i).getType()),
                                                 query.getColumn(i).getString());
                    queryFields.push_back(field);
                }
                results.push_back(queryFields);
            }
        }
        catch (const std::exception& e)
        {
            LogError("Error during Retrieve operation: {}.", e.what());
            throw;
        }
        return results;
    }

    void
    SQLiteManager::Remove(const std::string& tableName, const std::vector<Column>& selCriteria, LogicalOperator logOp)
    {
        std::string whereClause;
        if (!selCriteria.empty())
        {
            std::vector<std::string> critFields;
            for (auto& col : selCriteria)
            {
                if (col.m_type == ColumnType::TEXT)
                {
                    critFields.push_back(fmt::format("{}='{}'", col.m_name, col.m_value));
                }
                else
                {
                    critFields.push_back(fmt::format("{}={}", col.m_name, col.m_value));
                }
            }
            whereClause =
                fmt::format(" WHERE {}", fmt::join(critFields, fmt::format(" {} ", MAP_LOGOP_STRING.at(logOp))));
        }

        std::string queryString = fmt::format("DELETE FROM {}{}", tableName, whereClause);

        Execute(queryString);
    }

    void SQLiteManager::Update(const std::string& tableName,
                               const std::vector<Column>& fields,
                               const std::vector<Column>& selCriteria,
                               LogicalOperator logOp)
    {
        if (fields.empty())
        {
            LogError("Error: Missing update fields.");
            throw;
        }

        std::vector<std::string> setFields;
        for (auto& col : fields)
        {
            if (col.m_type == ColumnType::TEXT)
            {
                setFields.push_back(fmt::format("{}='{}'", col.m_name, col.m_value));
            }
            else
            {
                setFields.push_back(fmt::format("{}={}", col.m_name, col.m_value));
            }
        }
        std::string updateValues = fmt::format("{}", fmt::join(setFields, ", "));

        std::string whereClause;
        if (!selCriteria.empty())
        {
            std::vector<std::string> conditions;
            for (auto& col : selCriteria)
            {
                if (col.m_type == ColumnType::TEXT)
                {
                    conditions.push_back(fmt::format("{}='{}'", col.m_name, col.m_value));
                }
                else
                {
                    conditions.push_back(fmt::format("{}={}", col.m_name, col.m_value));
                }
            }
            whereClause =
                fmt::format(" WHERE {}", fmt::join(conditions, fmt::format(" {} ", MAP_LOGOP_STRING.at(logOp))));
        }

        std::string queryString = fmt::format("UPDATE {} SET {}{}", tableName, updateValues, whereClause);
        Execute(queryString);
    }

    void SQLiteManager::Execute(const std::string& query)
    {
        try
        {
            std::lock_guard<std::mutex> lock(m_mutex);
            m_db->exec(query);
        }
        catch (const std::exception& e)
        {
            LogError("Error during database operation: {}.", e.what());
            throw;
        }
    }

    void SQLiteManager::DropTable(const std::string& tableName)
    {
        std::string queryString = fmt::format("DROP TABLE {}", tableName);

        Execute(queryString);
    }

    SQLite::Transaction SQLiteManager::BeginTransaction()
    {
        return SQLite::Transaction(*m_db);
    }

    void SQLiteManager::CommitTransaction(SQLite::Transaction& transaction)
    {
        transaction.commit();
    }

    void SQLiteManager::RollbackTransaction(SQLite::Transaction& transaction)
    {
        transaction.rollback();
    }
} // namespace sqlite_manager
