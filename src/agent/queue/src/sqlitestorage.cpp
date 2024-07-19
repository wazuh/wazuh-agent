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
    : m_dbName(dbName), m_tableName(tableName), m_db(nullptr) {
    try {
        // open db
        m_db = std::make_unique<SQLite::Database>(m_dbName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        InitializeTable();
    } catch (const std::exception& e) {
        std::cerr << "Error initializing database: " << e.what() << std::endl;
        throw;
    }
}

SQLiteStorage::~SQLiteStorage() {
    // No need to explicitly close the database as std::unique_ptr will handle it
}

void SQLiteStorage::InitializeTable() {
    std::lock_guard<std::mutex> lock(m_mtx);
    try {
        std::string createTableQuery = CREATE_TABLE_QUERY;
        m_db->exec(createTableQuery);
    } catch (const std::exception& e) {
        std::cerr << "Error initializing table: " << e.what() << std::endl;
        throw;
    }
}

void SQLiteStorage::Store(const json& message) {
    std::lock_guard<std::mutex> lock(m_mtx);
    try {
        std::string insertQuery = INSERT_QUERY;
        SQLite::Statement query(*m_db, insertQuery);
        query.bind(1, message.dump());
        query.exec();
    } catch (const std::exception& e) {
        std::cerr << "Error storing message: " << e.what() << std::endl;
        throw;
    }
}

json SQLiteStorage::Retrieve(int id) {
    std::lock_guard<std::mutex> lock(m_mtx);
    try {
        std::string selectQuery = SELECT_QUERY;
        SQLite::Statement query(*m_db, selectQuery);
        query.bind(1, id);
        json message;
        if (query.executeStep()) {
            message = json::parse(query.getColumn(0).getString());
        } else {
            message = nullptr;
        }
        return message;
    } catch (const std::exception& e) {
        std::cerr << "Error retrieving message: " << e.what() << std::endl;
        throw;
    }
}

json SQLiteStorage::RetrieveMultiple(int n) {
    std::lock_guard<std::mutex> lock(m_mtx);
    try {
        std::string selectQuery = SELECT_MULTIPLE_QUERY;
        SQLite::Statement query(*m_db, selectQuery);
        query.bind(1, n);
        json messages = json::array();
        while (query.executeStep()) {
            messages.push_back(json::parse(query.getColumn(0).getString()));
        }
        std::reverse(messages.begin(), messages.end());
        return messages;
    } catch (const std::exception& e) {
        std::cerr << "Error retrieving multiple messages: " << e.what() << std::endl;
        throw;
    }
}

int SQLiteStorage::Remove(int id) {
    std::lock_guard<std::mutex> lock(m_mtx);
    try {
        std::string deleteQuery = DELETE_QUERY;
        SQLite::Statement query(*m_db, deleteQuery);
        query.bind(1, id);
        query.exec();
        return 1;
    } catch (const std::exception& e) {
        std::cerr << "Error removing message: " << e.what() << std::endl;
        throw;
    }
}

int SQLiteStorage::RemoveMultiple(int n) {
    std::lock_guard<std::mutex> lock(m_mtx);
    try {
        std::string deleteQuery = DELETE_MULTIPLE_QUERY;
        SQLite::Statement query(*m_db, deleteQuery);
        query.bind(1, n);
        query.exec();
        return n;
    } catch (const std::exception& e) {
        std::cerr << "Error removing multiple messages: " << e.what() << std::endl;
        throw;
    }
}

int SQLiteStorage::GetElementCount() {
    std::lock_guard<std::mutex> lock(m_mtx);
    try {
        std::string countQuery = COUNT_QUERY;
        SQLite::Statement query(*m_db, countQuery);
        int count = 0;
        if (query.executeStep()) {
            count = query.getColumn(0).getInt();
        }
        return count;
    } catch (const std::exception& e) {
        std::cerr << "Error getting element count: " << e.what() << std::endl;
        throw;
    }
}
