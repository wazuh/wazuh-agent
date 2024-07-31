#include "sqlitestorage.h"
#include <algorithm>
#include <iostream>

// TODO: replace define for constexpr and do only use memebers on runtime
#define CREATE_TABLE_QUERY    "CREATE TABLE IF NOT EXISTS " + m_tableName + " (message TEXT NOT NULL);"
#define INSERT_QUERY          "INSERT INTO " + m_tableName + " (message) VALUES (?);"
#define SELECT_QUERY          "SELECT message FROM " + m_tableName + " WHERE rowid = ?;"
#define SELECT_MULTIPLE_QUERY "SELECT message FROM " + m_tableName + " ORDER BY rowid ASC LIMIT ?;"
#define DELETE_QUERY          "DELETE FROM " + m_tableName + " WHERE rowid = ?;"
#define DELETE_MULTIPLE_QUERY                                                                                          \
    "DELETE FROM " + m_tableName + " WHERE rowid IN (SELECT rowid FROM " + m_tableName + " ORDER BY rowid ASC LIMIT ?);"
#define COUNT_QUERY "SELECT COUNT(*) FROM " + m_tableName + ";"

SQLiteStorage::SQLiteStorage(const std::string& dbName, const std::string& tableName)
    : m_dbName(dbName)
    , m_tableName(tableName)
    , m_db(make_unique<SQLite::Database>(dbName + tableName + ".db", SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE))
{
    try
    {
        // Open the database in WAL mode
        m_db->exec("PRAGMA journal_mode=WAL;");
        InitializeTable();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error initializing database: " << e.what() << std::endl;
        throw;
    }
}

SQLiteStorage::~SQLiteStorage() {}

void SQLiteStorage::InitializeTable()
{
    std::lock_guard<std::mutex> lock(m_mutex);
    try
    {
        std::string createTableQuery = CREATE_TABLE_QUERY;
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

void SQLiteStorage::Store(const json& message)
{
    waitForDatabaseAccess();

    std::string insertQuery = INSERT_QUERY;
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

json SQLiteStorage::Retrieve(int id)
{
    try
    {
        std::string selectQuery = SELECT_QUERY;
        SQLite::Statement query(*m_db, selectQuery);
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

json SQLiteStorage::RetrieveMultiple(int n)
{
    try
    {
        std::string selectQuery = SELECT_MULTIPLE_QUERY;
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

int SQLiteStorage::Remove(int id)
{
    try
    {
        std::string deleteQuery = DELETE_QUERY;
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

int SQLiteStorage::RemoveMultiple(int n)
{
    try
    {
        waitForDatabaseAccess();
        SQLite::Transaction transaction(*m_db);
        std::string deleteQuery = DELETE_MULTIPLE_QUERY;
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

int SQLiteStorage::GetElementCount()
{
    try
    {
        std::string countQuery = COUNT_QUERY;
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
