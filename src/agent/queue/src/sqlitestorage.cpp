#include "sqlitestorage.h"
#include <algorithm>
// TODO: only in gcc13, does it worth it? #include <format>
#include <fmt/format.h>
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
    constexpr std::string_view CREATE_TABLE_QUERY {"CREATE TABLE IF NOT EXISTS {} (message TEXT NOT NULL);"};
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
    m_cv.wait(lock, [this] { return !m_db_in_use; });
    m_db_in_use = true;
}

void SQLiteStorage::releaseDatabaseAccess()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    m_db_in_use = false;
    m_cv.notify_one();
}

void SQLiteStorage::Store(const json& message, const std::string& tableName)
{
    waitForDatabaseAccess();
    constexpr std::string_view INSERT_QUERY {"INSERT INTO {} (message) VALUES (?);"};
    std::string insertQuery = fmt::format(INSERT_QUERY, tableName);
    SQLite::Statement query = SQLite::Statement(*m_db, insertQuery);

    if (message.is_array())
    {
        SQLite::Transaction transaction(*m_db);
        for (const auto& singleMessageData : message)
        {
            try
            {
                query.bind(1, singleMessageData.dump());
                query.exec();
            }
            catch (const SQLite::Exception& e)
            {
                std::cerr << "2 " << e.what() << '\n';
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
        query.exec();
        transaction.commit();
    }
    releaseDatabaseAccess();
}

json SQLiteStorage::Retrieve(int id, const std::string& tableName)
{
    try
    {
        constexpr std::string_view SELECT_QUERY {"SELECT message FROM {} WHERE rowid = ?;"};
        std::string selectQuery = fmt::format(SELECT_QUERY, tableName);
        SQLite::Statement query(*m_db, selectQuery);
        // TODO: delte id?
        query.bind(1, id);
        json message;
        if (query.executeStep())
        {
            message = json::parse(query.getColumn(0).getString());
        }
        else
        {
            message = nullptr;
        }
        return message;
    }
    catch (const SQLite::Exception& e)
    {
        std::cerr << "Error retrieving message: " << e.what() << std::endl;
        throw;
    }
}

json SQLiteStorage::RetrieveMultiple(int n, const std::string& tableName)
{
    try
    {
        constexpr std::string_view SELECT_MULTIPLE_QUERY {"SELECT message FROM {} ORDER BY rowid ASC LIMIT ?;"};
        std::string selectQuery = fmt::format(SELECT_MULTIPLE_QUERY, tableName);
        SQLite::Statement query(*m_db, selectQuery);
        query.bind(1, n);
        json messages = json::array();
        while (query.executeStep())
        {
            messages.push_back(json::parse(query.getColumn(0).getString()));
        }
        std::reverse(messages.begin(), messages.end());
        return messages;
    }
    catch (const SQLite::Exception& e)
    {
        std::cerr << "Error retrieving multiple messages: " << e.what() << std::endl;
        throw;
    }
}

int SQLiteStorage::Remove(int id, const std::string& tableName)
{
    constexpr std::string_view DELETE_QUERY {"DELETE FROM {} WHERE rowid = ?;"};
    try
    {
        std::string deleteQuery = fmt::format(DELETE_QUERY, tableName);
        SQLite::Statement query(*m_db, deleteQuery);
        query.bind(1, id);
        SQLite::Transaction transaction(*m_db);
        query.exec();
        transaction.commit();
        return 1;
    }
    catch (const SQLite::Exception& e)
    {
        std::cerr << "Error removing message: " << e.what() << std::endl;
        throw;
    }
}

int SQLiteStorage::RemoveMultiple(int n, const std::string& tableName)
{
    constexpr std::string_view DELETE_MULTIPLE_QUERY {
        "DELETE FROM {} WHERE rowid IN (SELECT rowid FROM {} ORDER BY rowid ASC LIMIT ?);"};
    try
    {
        waitForDatabaseAccess();
        SQLite::Transaction transaction(*m_db);
        std::string deleteQuery = fmt::format(DELETE_MULTIPLE_QUERY, tableName, tableName);
        SQLite::Statement query(*m_db, deleteQuery);
        query.bind(1, n);
        query.exec();
        transaction.commit();
        releaseDatabaseAccess();
        return n;
    }
    catch (const SQLite::Exception& e)
    {
        std::cerr << "Error removing multiple messages: " << e.what() << std::endl;
        throw;
    }
}

int SQLiteStorage::GetElementCount(const std::string& tableName)
{
    constexpr std::string_view COUNT_QUERY {"SELECT COUNT(*) FROM {}"};
    try
    {
        std::string countQuery = fmt::format(COUNT_QUERY, tableName);
        SQLite::Statement query(*m_db, countQuery);
        int count = 0;
        if (query.executeStep())
        {
            count = query.getColumn(0).getInt();
        }
        return count;
    }
    catch (const SQLite::Exception& e)
    {
        std::cerr << "Error getting element count: " << e.what() << std::endl;
        throw;
    }
}
