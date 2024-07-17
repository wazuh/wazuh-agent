#include "sqlitestorage.h"
#include <iostream>
#include <algorithm>


#define CREATE_TABLE_QUERY "CREATE TABLE IF NOT EXISTS " + m_tableName + " (message TEXT NOT NULL);"
#define INSERT_QUERY "INSERT INTO " + m_tableName + " (message) VALUES (?);"
#define SELECT_QUERY "SELECT message FROM " + m_tableName + " WHERE rowid = ?;"
#define SELECT_MULTIPLE_QUERY "SELECT message FROM " + m_tableName + " ORDER BY rowid ASC LIMIT ?;"
#define DELETE_QUERY "DELETE FROM " + m_tableName + " WHERE rowid = ?;"
#define DELETE_MULTIPLE_QUERY "DELETE FROM " + m_tableName + " WHERE rowid IN (SELECT rowid FROM " + m_tableName + " ORDER BY rowid ASC LIMIT ?);"
#define COUNT_QUERY "SELECT COUNT(*) FROM " + m_tableName + ";"

SQLiteStorage::SQLiteStorage(const std::string& dbName, const std::string& tableName)
    : m_dbName(dbName), m_tableName(tableName), m_db(nullptr)  {
    OpenDB();
    InitializeTable();
    CloseDB();
}

SQLiteStorage::~SQLiteStorage() {
    // Destructor vacío ya que la base de datos se cerrará después de cada operación
}

void SQLiteStorage::InitializeTable() {
    std::string createTableQuery = CREATE_TABLE_QUERY;
    sqlite3_exec(m_db, createTableQuery.c_str(), 0, 0, 0);
}

void SQLiteStorage::OpenDB() {
    sqlite3_open(m_dbName.c_str(), &m_db);
}

void SQLiteStorage::CloseDB() {
    if (m_db) {
        sqlite3_close(m_db);
        m_db = nullptr;
    }
}

void SQLiteStorage::Store(const json& message) {
    std::lock_guard<std::mutex> lock(m_mtx);
    std::string insertQuery = INSERT_QUERY;
    OpenDB();
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_db, insertQuery.c_str(), -1, &stmt, 0);
    if(message.is_array())
    {
        for (auto& singleMessageData : message.items())
        {
            std::string messageStr = singleMessageData.value();
            sqlite3_bind_text(stmt, 1, messageStr.c_str(), -1, SQLITE_TRANSIENT);
            sqlite3_step(stmt);
        }
    }
    else
    {
        std::string messageStr = message.dump();
        sqlite3_bind_text(stmt, 1, messageStr.c_str(), -1, SQLITE_TRANSIENT);
        sqlite3_step(stmt);
    }
    sqlite3_finalize(stmt);
    CloseDB();
}

json SQLiteStorage::Retrieve(int id) {
    std::lock_guard<std::mutex> lock(m_mtx);
    OpenDB();

    std::string selectQuery = SELECT_QUERY;
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_db, selectQuery.c_str(), -1, &stmt, 0);
    sqlite3_bind_int(stmt, 1, id);
    json message;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string messageStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        message = json::parse(messageStr);
    } else {
        message = nullptr;
    }
    sqlite3_finalize(stmt);

    CloseDB();
    return message;
}

json SQLiteStorage::RetrieveMultiple(int n) {
    std::lock_guard<std::mutex> lock(m_mtx);
    OpenDB();

    std::string selectQuery = SELECT_MULTIPLE_QUERY;
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_db, selectQuery.c_str(), -1, &stmt, 0);
    sqlite3_bind_int(stmt, 1, n);
    json messages = json::array();
    while (sqlite3_step(stmt) == SQLITE_ROW) {
        std::string messageStr = reinterpret_cast<const char*>(sqlite3_column_text(stmt, 0));
        messages.push_back(json::parse(messageStr));
    }
    sqlite3_finalize(stmt);

    CloseDB();
    return messages;
}

int SQLiteStorage::Remove(int id) {
    std::lock_guard<std::mutex> lock(m_mtx);
    OpenDB();

    std::string deleteQuery = DELETE_QUERY;
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_db, deleteQuery.c_str(), -1, &stmt, 0);
    sqlite3_bind_int(stmt, 1, id);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    CloseDB();

    return 1;
}

int SQLiteStorage::RemoveMultiple(int n) {
    std::lock_guard<std::mutex> lock(m_mtx);
    OpenDB();

    std::string deleteQuery = DELETE_MULTIPLE_QUERY;
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_db, deleteQuery.c_str(), -1, &stmt, 0);
    sqlite3_bind_int(stmt, 1, n);
    sqlite3_step(stmt);
    sqlite3_finalize(stmt);

    CloseDB();

    return n;
}

int SQLiteStorage::GetElementCount() {
    std::lock_guard<std::mutex> lock(m_mtx);
    OpenDB();

    std::string countQuery = COUNT_QUERY;
    sqlite3_stmt* stmt;
    sqlite3_prepare_v2(m_db, countQuery.c_str(), -1, &stmt, 0);
    int count = 0;
    if (sqlite3_step(stmt) == SQLITE_ROW) {
        count = sqlite3_column_int(stmt, 0);
    }
    sqlite3_finalize(stmt);

    CloseDB();
    return count;
}
