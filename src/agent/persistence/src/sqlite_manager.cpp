#include <sqlite_manager.hpp>

#include <logger.hpp>

#include <SQLiteCpp/SQLiteCpp.h>
#include <fmt/format.h>
#include <map>
#include <regex>

const std::map<ColumnType, std::string> MAP_COL_TYPE_STRING {
    {ColumnType::INTEGER, "INTEGER"}, {ColumnType::TEXT, "TEXT"}, {ColumnType::REAL, "REAL"}};
const std::map<LogicalOperator, std::string> MAP_LOGOP_STRING {{LogicalOperator::AND, "AND"},
                                                               {LogicalOperator::OR, "OR"}};
const std::map<OrderType, std::string> MAP_ORDER_STRING {{OrderType::ASC, "ASC"}, {OrderType::DESC, "DESC"}};

SQLiteManager::~SQLiteManager() = default;

namespace
{
    const std::string& TO_SEARCH = "'";
    const std::string& TO_REPLACE = "''";

    /// @brief Escapes single quotes in a string.
    std::string EscapeSingleQuotes(const std::string& str)
    {
        return std::regex_replace(str, std::regex(TO_SEARCH), TO_REPLACE);
    }
} // namespace

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

bool SQLiteManager::TableExists(const std::string& table)
{
    try
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        SQLite::Statement query(*m_db, "SELECT name FROM sqlite_master WHERE type='table' AND name='" + table + "';");
        return query.executeStep();
    }
    catch (const std::exception& e)
    {
        LogError("Failed to check if table exists: {}.", e.what());
        return false;
    }
}

void SQLiteManager::CreateTable(const std::string& tableName, const Keys& cols)
{
    std::vector<std::string> pk;
    std::vector<std::string> fields;
    for (const auto& col : cols)
    {
        std::string field = fmt::format(
            "{} {}{}", col.Name, MAP_COL_TYPE_STRING.at(col.Type), (col.Attributes & NOT_NULL) ? " NOT NULL" : "");
        if (col.Attributes & PRIMARY_KEY)
        {
            pk.push_back((col.Attributes & AUTO_INCREMENT) ? fmt::format("{} AUTOINCREMENT", col.Name) : col.Name);
        }
        fields.push_back(field);
    }

    std::string primaryKey = pk.empty() ? "" : fmt::format(", PRIMARY KEY({})", fmt::join(pk, ", "));
    std::string queryString = fmt::format(
        "CREATE TABLE IF NOT EXISTS {} ({}{})", tableName, fmt::format("{}", fmt::join(fields, ", ")), primaryKey);

    Execute(queryString);
}

void SQLiteManager::Insert(const std::string& tableName, const Row& cols)
{
    std::vector<std::string> names;
    std::vector<std::string> values;

    for (const auto& col : cols)
    {
        names.push_back(col.Name);
        if (col.Type == ColumnType::TEXT)
        {
            auto escapedValue = EscapeSingleQuotes(col.Value);
            values.push_back(fmt::format("'{}'", escapedValue));
        }
        else
        {
            values.push_back(col.Value);
        }
    }

    std::string queryString =
        fmt::format("INSERT INTO {} ({}) VALUES ({})", tableName, fmt::join(names, ", "), fmt::join(values, ", "));

    Execute(queryString);
}

void SQLiteManager::Update(const std::string& tableName,
                           const Row& fields,
                           const Criteria& selCriteria,
                           LogicalOperator logOp)
{
    if (fields.empty())
    {
        LogError("Error: Missing update fields.");
        throw;
    }

    std::vector<std::string> setFields;
    for (const auto& col : fields)
    {
        if (col.Type == ColumnType::TEXT)
        {
            auto escapedValue = EscapeSingleQuotes(col.Value);
            setFields.push_back(fmt::format("{}='{}'", col.Name, escapedValue));
        }
        else
        {
            setFields.push_back(fmt::format("{}={}", col.Name, col.Value));
        }
    }
    std::string updateValues = fmt::format("{}", fmt::join(setFields, ", "));

    std::string whereClause;
    if (!selCriteria.empty())
    {
        std::vector<std::string> conditions;
        for (const auto& col : selCriteria)
        {
            if (col.Type == ColumnType::TEXT)
            {
                auto escapedValue = EscapeSingleQuotes(col.Value);
                conditions.push_back(fmt::format("{}='{}'", col.Name, escapedValue));
            }
            else
            {
                conditions.push_back(fmt::format("{}={}", col.Name, col.Value));
            }
        }
        whereClause = fmt::format(" WHERE {}", fmt::join(conditions, fmt::format(" {} ", MAP_LOGOP_STRING.at(logOp))));
    }

    std::string queryString = fmt::format("UPDATE {} SET {}{}", tableName, updateValues, whereClause);

    Execute(queryString);
}

void SQLiteManager::Remove(const std::string& tableName, const Criteria& selCriteria, LogicalOperator logOp)
{
    std::string whereClause;
    if (!selCriteria.empty())
    {
        std::vector<std::string> critFields;
        for (const auto& col : selCriteria)
        {
            if (col.Type == ColumnType::TEXT)
            {
                auto escapedValue = EscapeSingleQuotes(col.Value);
                critFields.push_back(fmt::format("{}='{}'", col.Name, escapedValue));
            }
            else
            {
                critFields.push_back(fmt::format("{}={}", col.Name, col.Value));
            }
        }
        whereClause = fmt::format(" WHERE {}", fmt::join(critFields, fmt::format(" {} ", MAP_LOGOP_STRING.at(logOp))));
    }

    std::string queryString = fmt::format("DELETE FROM {}{}", tableName, whereClause);

    Execute(queryString);
}

void SQLiteManager::DropTable(const std::string& tableName)
{
    std::string queryString = fmt::format("DROP TABLE {}", tableName);

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

std::vector<Row> SQLiteManager::Select(const std::string& tableName,
                                       const Names& fields,
                                       const Criteria& selCriteria,
                                       LogicalOperator logOp,
                                       const Names& orderBy,
                                       OrderType orderType,
                                       int limit)
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

        for (const auto& col : fields)
        {
            fieldNames.push_back(col.Name);
        }
        selectedFields = fmt::format("{}", fmt::join(fieldNames, ", "));
    }

    std::string condition;
    if (!selCriteria.empty())
    {
        std::vector<std::string> conditions;
        for (const auto& col : selCriteria)
        {
            if (col.Type == ColumnType::TEXT)
            {
                auto escapedValue = EscapeSingleQuotes(col.Value);
                conditions.push_back(fmt::format("{}='{}'", col.Name, escapedValue));
            }
            else
            {
                conditions.push_back(fmt::format("{}={}", col.Name, col.Value));
            }
        }
        condition = fmt::format("WHERE {}", fmt::join(conditions, fmt::format(" {} ", MAP_LOGOP_STRING.at(logOp))));
    }

    if (!orderBy.empty())
    {
        std::vector<std::string> orderFields;
        orderFields.reserve(orderBy.size());
        for (const auto& col : orderBy)
        {
            orderFields.push_back(col.Name);
        }
        condition += fmt::format(" ORDER BY {}", fmt::join(orderFields, ", "));
        condition += fmt::format(" {}", MAP_ORDER_STRING.at(orderType));
    }

    if (limit > 0)
    {
        condition += fmt::format(" LIMIT {}", limit);
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
            Row queryFields;
            queryFields.reserve(static_cast<size_t>(nColumns));
            for (int i = 0; i < nColumns; i++)
            {
                queryFields.emplace_back(query.getColumn(i).getName(),
                                         ColumnTypeFromSQLiteType(query.getColumn(i).getType()),
                                         query.getColumn(i).getString());
            }
            results.push_back(queryFields);
        }
    }
    catch (const std::exception& e)
    {
        LogError("Error during Select operation: {}.", e.what());
        throw;
    }
    return results;
}

int SQLiteManager::GetCount(const std::string& tableName, const Criteria& selCriteria, LogicalOperator logOp)
{
    std::string condition;
    if (!selCriteria.empty())
    {
        std::vector<std::string> conditions;
        for (const auto& col : selCriteria)
        {
            if (col.Type == ColumnType::TEXT)
            {
                auto escapedValue = EscapeSingleQuotes(col.Value);
                conditions.push_back(fmt::format("{}='{}'", col.Name, escapedValue));
            }
            else
            {
                conditions.push_back(fmt::format("{}={}", col.Name, col.Value));
            }
        }
        condition = fmt::format("WHERE {}", fmt::join(conditions, fmt::format(" {} ", MAP_LOGOP_STRING.at(logOp))));
    }

    std::string queryString = fmt::format("SELECT COUNT(*) FROM {} {}", tableName, condition);

    int count = 0;
    try
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        SQLite::Statement query(*m_db, queryString);

        if (query.executeStep())
        {
            count = query.getColumn(0).getInt();
        }
        else
        {
            LogError("Error getting element count.");
        }
    }
    catch (const std::exception& e)
    {
        LogError("Error during GetCount operation: {}.", e.what());
        throw;
    }
    return count;
}

size_t SQLiteManager::GetSize(const std::string& tableName,
                              const Names& fields,
                              const Criteria& selCriteria,
                              LogicalOperator logOp)
{
    if (fields.empty())
    {
        LogError("Error: Missing size fields.");
        throw;
    }

    std::string selectedFields;
    std::vector<std::string> fieldNames;
    fieldNames.reserve(fields.size());

    for (const auto& col : fields)
    {
        fieldNames.push_back("LENGTH(" + col.Name + ")");
    }
    selectedFields = fmt::format("{}", fmt::join(fieldNames, " + "));

    std::string condition;
    if (!selCriteria.empty())
    {
        std::vector<std::string> conditions;
        for (const auto& col : selCriteria)
        {
            if (col.Type == ColumnType::TEXT)
            {
                auto escapedValue = EscapeSingleQuotes(col.Value);
                conditions.push_back(fmt::format("{}='{}'", col.Name, escapedValue));
            }
            else
            {
                conditions.push_back(fmt::format("{}={}", col.Name, col.Value));
            }
        }
        condition = fmt::format("WHERE {}", fmt::join(conditions, fmt::format(" {} ", MAP_LOGOP_STRING.at(logOp))));
    }

    std::string queryString =
        fmt::format("SELECT SUM({}) AS total_bytes FROM {} {}", selectedFields, tableName, condition);

    size_t count = 0;
    try
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        SQLite::Statement query(*m_db, queryString);

        if (query.executeStep())
        {
            count = query.getColumn(0).getUInt();
        }
        else
        {
            LogError("Error getting element count.");
        }
    }
    catch (const std::exception& e)
    {
        LogError("Error during GetSize operation: {}.", e.what());
        throw;
    }
    return count;
}

TransactionId SQLiteManager::BeginTransaction()
{
    TransactionId transactionId = m_nextTransactionId++;
    m_transactions.emplace(transactionId, std::make_unique<SQLite::Transaction>(*m_db));
    return transactionId;
}

void SQLiteManager::CommitTransaction(TransactionId transactionId)
{
    m_transactions.at(transactionId)->commit();
    m_transactions.erase(transactionId);
}

void SQLiteManager::RollbackTransaction(TransactionId transactionId)
{
    m_transactions.at(transactionId)->rollback();
    m_transactions.erase(transactionId);
}
