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

    void SQLiteManager::Insert(const std::string& tableName, const std::vector<Col>& cols)
    {
        std::vector<std::string> names;
        std::vector<std::string> values;

        for (const Col& col : cols)
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
            format("INSERT INTO {} ({}) VALUES ({})", tableName, fmt::join(names, ", "), fmt::join(values, ", "));

        std::lock_guard<std::mutex> lock(m_mutex);
        try
        {
            m_db->exec(queryString);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error inserting element: " << e.what() << std::endl;
            throw;
        }
    }

    void SQLiteManager::ExecuteNoSelectSQL(const std::string& queryString)
    {
        std::lock_guard<std::mutex> lock(m_mutex);
        try
        {
            m_db->exec(queryString);
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error executing SQL: " << e.what() << std::endl;
            throw;
        }
    }
} // namespace sqlite_manager
