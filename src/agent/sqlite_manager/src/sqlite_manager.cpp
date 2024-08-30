#include <sqlite_manager.hpp>

#include <fmt/format.h>
#include <iostream>
#include <map>

namespace sqlite_manager
{
    const std::map<ColumnType, std::string> mapColTypeString {
        {ColumnType::INTEGER, "INTEGER"}, {ColumnType::TEXT, "TEXT"}, {ColumnType::FLOAT, "REAL"}};

    ColumnType SQLiteManager::ColumnTypeFromSQLiteType(const int type) const
    {
        if (type == SQLite::INTEGER)
            return ColumnType::INTEGER;

        if (type == SQLite::FLOAT)
            return ColumnType::FLOAT;

        return ColumnType::TEXT;
    }

    SQLiteManager::SQLiteManager(const std::string& dbName)
        : m_dbName(dbName)
        , m_db(std::make_unique<SQLite::Database>(dbName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE))
    {
        m_db->exec("PRAGMA journal_mode=WAL;");
    }

    void SQLiteManager::CreateTable(const std::string& tableName, const std::vector<Col>& cols)
    {
        std::vector<std::string> fields;
        for (const Col& col : cols)
        {
            std::string field = fmt::format("{} {}{}{}{}",
                                            col.m_name,
                                            mapColTypeString.at(col.m_type),
                                            (col.m_primaryKey) ? " PRIMARY KEY" : "",
                                            (col.m_autoIncrement) ? " AUTOINCREMENT" : "",
                                            (col.m_notNull) ? " NOT NULL" : "");
            fields.push_back(field);
        }

        std::string queryString =
            fmt::format("CREATE TABLE IF NOT EXISTS {} ({})", tableName, fmt::format("{}", fmt::join(fields, ", ")));

        std::lock_guard<std::mutex> lock(m_mutex);
        try
        {
            m_db->exec(queryString);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error initializing table: " << e.what() << std::endl;
            throw;
        }
    }
} // namespace sqlite_manager
