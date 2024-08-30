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
                std::cerr << "Error getting element count." << std::endl;
            }
            return count;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error during GetCount operation: " << e.what() << std::endl;
            return {};
        }
    }

    std::vector<Row> SQLiteManager::Select(const std::string& tableName,
                                           const std::vector<Col>& fields,
                                           const std::vector<Col>& selCriteria)
    {
        std::string selectedFields;
        if (fields.size() == 0)
        {
            selectedFields = "*";
        }
        else
        {
            std::vector<std::string> fieldNames;
            for (auto& col : fields)
            {
                fieldNames.push_back(col.m_name);
            }
            selectedFields = fmt::format("{}", fmt::join(fieldNames, ", "));
        }

        std::string condition;
        if (selCriteria.size() != 0)
        {
            std::vector<std::string> conditions;
            for (auto& col : selCriteria)
            {
                if (col.m_type == ColumnType::TEXT)
                    conditions.push_back(fmt::format("{} = '{}'", col.m_name, col.m_value));
                else
                    conditions.push_back(fmt::format("{} = {}", col.m_name, col.m_value));
            }
            condition = fmt::format("WHERE {}", fmt::join(conditions, " AND "));
        }

        std::string queryString = fmt::format("SELECT {} FROM {} {}", selectedFields, tableName, condition);

        std::cout << "QueryString: " << queryString << std::endl;

        // Do the actual query
        std::vector<Row> results;
        try
        {
            std::lock_guard<std::mutex> lock(m_mutex);

            SQLite::Statement query(*m_db, queryString);
            while (query.executeStep())
            {
                int nColumns = query.getColumnCount();
                std::vector<Col> fields;
                fields.reserve(nColumns);
                for (int i = 0; i < nColumns; i++)
                {
                    sqlite_manager::Col field(query.getColumn(i).getName(),
                                              ColumnTypeFromSQLiteType(query.getColumn(i).getType()),
                                              query.getColumn(i).getString());
                    fields.push_back(field);
                }
                results.push_back(fields);
            }

            return results;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error during Retrieve operation: " << e.what() << std::endl;
            return {};
        }
        return results;
    }
} // namespace sqlite_manager
