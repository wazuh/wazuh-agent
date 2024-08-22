#include <sqlitestorage.hpp>

#include <fmt/format.h>

#include <algorithm>
#include <exception>
#include <iostream>

SQLiteStorage::SQLiteStorage(const std::string& dbName, const std::vector<std::string> tableNames)
    : m_dbName(dbName)
    , m_db(make_unique<SQLite::Database>(dbName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE))
{
    try
    {
        // Open the database in WAL mode
        m_db->exec("PRAGMA journal_mode=WAL;");
        for (auto table : tableNames)
        {
            InitializeTable(table);
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error initializing database: " << e.what() << std::endl;
        throw;
    }
}

SQLiteStorage::~SQLiteStorage() {}

void SQLiteStorage::InitializeTable(const std::string& tableName)
{
    // TODO: all queries should be in the same place.
    constexpr std::string_view CREATE_TABLE_QUERY {
        "CREATE TABLE IF NOT EXISTS {} (module TEXT, message TEXT NOT NULL);"};
    auto createTableQuery = fmt::format(CREATE_TABLE_QUERY, tableName);
    std::lock_guard<std::mutex> lock(m_mutex);
    try
    {
        m_db->exec(createTableQuery);
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error initializing table: " << e.what() << std::endl;
        throw;
    }
}

void SQLiteStorage::waitForDatabaseAccess()
{
    std::unique_lock<std::mutex> lock(m_mutex);
    m_cv.wait(lock, [this] { return !m_dbInUse; });
    m_dbInUse = true;
}

void SQLiteStorage::releaseDatabaseAccess()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_dbInUse = false;
    m_cv.notify_one();
}

int SQLiteStorage::Store(const json& message, const std::string& tableName, const std::string& moduleName)
{
    constexpr std::string_view INSERT_QUERY {"INSERT INTO {} (module, message) VALUES (\"{}\", ?);"};
    std::string insertQuery = fmt::format(INSERT_QUERY, tableName, moduleName);
    int result = 0;

    waitForDatabaseAccess();
    SQLite::Statement query = SQLite::Statement(*m_db, insertQuery);

    if (message.is_array())
    {
        SQLite::Transaction transaction(*m_db);
        for (const auto& singleMessageData : message)
        {
            try
            {
                query.bind(1, singleMessageData.dump());
                result += query.exec();
            }
            catch (const std::exception& e)
            {
                std::cerr << "Error during Store operation: " << e.what() << '\n';
                break;
            }
            // Reset the query to reuse it for the next message
            query.reset();
        }
        transaction.commit();
    }
    else
    {
        SQLite::Transaction transaction(*m_db);
        query.bind(1, message.dump());
        result = query.exec();
        transaction.commit();
    }
    releaseDatabaseAccess();

    return result;
}

// TODO: we shouldn't use rowid outside the table itself
json SQLiteStorage::Retrieve(int id, const std::string& tableName, const std::string& moduleName)
{

    std::string selectQuery;
    if (moduleName.empty())
    {
        constexpr std::string_view SELECT_QUERY {"SELECT module, message FROM {} WHERE rowid = ?;"};
        selectQuery = fmt::format(SELECT_QUERY, tableName);
    }
    else
    {
        constexpr std::string_view SELECT_QUERY {
            "SELECT module, message FROM {} WHERE module LIKE \"{}\" AND rowid = ?;"};
        selectQuery = fmt::format(SELECT_QUERY, tableName, moduleName);
    }

    try
    {
        SQLite::Statement query(*m_db, selectQuery);
        query.bind(1, id);
        json outputJson = {{"module", ""}, {"data", {}}};
        if (query.executeStep())
        {
            std::string dataString;
            std::string moduleString;

            if (query.getColumnCount() == 2 && query.getColumn(1).getType() == SQLite::TEXT &&
                query.getColumn(0).getType() == SQLite::TEXT)
            {
                moduleString = query.getColumn(0).getString();
                dataString = query.getColumn(1).getString();

                if (!dataString.empty())
                {
                    outputJson["data"] = json::parse(dataString);
                }

                if (!moduleString.empty())
                {
                    outputJson["module"] = moduleString;
                }
            }
        }

        return outputJson;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error during Retrieve operation: " << e.what() << std::endl;
        return {};
    }
}

json SQLiteStorage::RetrieveMultiple(int n, const std::string& tableName, const std::string& moduleName)
{
    std::string selectQuery;
    if (moduleName.empty())
    {
        constexpr std::string_view SELECT_MULTIPLE_QUERY {"SELECT module, message FROM {} ORDER BY rowid ASC LIMIT ?;"};
        selectQuery = fmt::format(SELECT_MULTIPLE_QUERY, tableName);
    }
    else
    {
        constexpr std::string_view SELECT_MULTIPLE_QUERY {
            "SELECT module, message FROM {} WHERE module LIKE \"{}\" ORDER BY rowid ASC LIMIT ?;"};
        selectQuery = fmt::format(SELECT_MULTIPLE_QUERY, tableName, moduleName);
    }

    try
    {
        SQLite::Statement query(*m_db, selectQuery);
        query.bind(1, n);
        json messages = json::array();
        while (query.executeStep())
        {
            // getting data json
            std::string dataString;
            std::string moduleString;

            if (query.getColumnCount() == 2 && query.getColumn(1).getType() == SQLite::TEXT &&
                query.getColumn(0).getType() == SQLite::TEXT)
            {
                moduleString = query.getColumn(0).getString();
                dataString = query.getColumn(1).getString();

                json outputJson = {{"module", ""}, {"data", {}}};

                if (!dataString.empty())
                {
                    outputJson["data"] = json::parse(dataString);
                }

                if (!moduleString.empty())
                {
                    outputJson["module"] = moduleString;
                }

                messages.push_back(outputJson);
            }
        }

        return messages;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error during RetrieveMultiple operation: " << e.what() << std::endl;
        return {};
    }
}

int SQLiteStorage::Remove(int id, const std::string& tableName, const std::string& moduleName)
{
    std::string deleteQuery;
    if (moduleName.empty())
    {
        constexpr std::string_view DELETE_QUERY {"DELETE FROM {} WHERE rowid = ?;"};
        deleteQuery = fmt::format(DELETE_QUERY, tableName);
    }
    else
    {
        constexpr std::string_view DELETE_QUERY {"DELETE FROM {} WHERE module LIKE \"{}\" AND rowid = ?;"};
        deleteQuery = fmt::format(DELETE_QUERY, tableName, moduleName);
    }

    try
    {
        SQLite::Statement query(*m_db, deleteQuery);
        query.bind(1, id);
        SQLite::Transaction transaction(*m_db);
        query.exec();
        transaction.commit();
        return 1;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error during Remove operation: " << e.what() << std::endl;
        return {};
    }
}

int SQLiteStorage::RemoveMultiple(int n, const std::string& tableName, const std::string& moduleName)
{
    std::string deleteQuery;
    int rowsModified = 0;
    if (moduleName.empty())
    {
        constexpr std::string_view DELETE_MULTIPLE_QUERY {
            "DELETE FROM {} WHERE rowid IN (SELECT rowid FROM {} ORDER BY rowid ASC LIMIT ?);"};
        deleteQuery = fmt::format(DELETE_MULTIPLE_QUERY, tableName, tableName);
    }
    else
    {
        constexpr std::string_view DELETE_MULTIPLE_QUERY {
            "DELETE FROM {} WHERE module LIKE \"{}\" AND rowid IN (SELECT rowid FROM {} WHERE module LIKE \"{}\" ORDER "
            "BY rowid ASC LIMIT ?);"};
        deleteQuery = fmt::format(DELETE_MULTIPLE_QUERY, tableName, moduleName, tableName, moduleName);
    }

    try
    {
        waitForDatabaseAccess();
        SQLite::Statement query(*m_db, deleteQuery);
        SQLite::Transaction transaction(*m_db);
        query.bind(1, n);
        rowsModified = query.exec();
        transaction.commit();
        releaseDatabaseAccess();
        return rowsModified;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error during RemoveMultiple operation: " << e.what() << std::endl;
        return rowsModified;
    }
}

int SQLiteStorage::GetElementCount(const std::string& tableName, const std::string& moduleName)
{
    std::string countQuery;
    if (moduleName.empty())
    {
        constexpr std::string_view COUNT_QUERY {"SELECT COUNT(*) FROM {}"};
        countQuery = fmt::format(COUNT_QUERY, tableName);
    }
    else
    {
        constexpr std::string_view COUNT_QUERY {"SELECT COUNT(*) FROM {} WHERE module LIKE \"{}\""};
        countQuery = fmt::format(COUNT_QUERY, tableName, moduleName);
    }

    try
    {
        SQLite::Statement query(*m_db, countQuery);
        int count = 0;
        if (query.executeStep())
        {
            count = query.getColumn(0).getInt();
        }
        else
        {
            std::cerr << "Error SQLiteStorage get element count." << std::endl;
        }
        return count;
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error during GetElementCount operation: " << e.what() << std::endl;
        return {};
    }
}
