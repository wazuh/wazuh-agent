#pragma ocne

#include "db_wrapper.hpp"

#include <iostream>
#include <sqlite3.h>

static const char* sqlitedb_path = "sqlite3_events.db";

class SQLiteWrapper : public DBWrapper
{
public:
    SQLiteWrapper()
    {
        if (sqlite3_open(sqlitedb_path, &db))
        {
            std::cerr << "Can't open database: " << sqlite3_errmsg(db) << std::endl;
            db = nullptr;
        }
    }

    ~SQLiteWrapper() override
    {
        if (db)
        {
            sqlite3_close(db);
        }
    }

    void DropTable()
    {
        const char* sql = "DROP TABLE IF EXISTS events;";
        char* errMsg = nullptr;
        int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK)
        {
            std::cerr << "Error creating table: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        } 
    }

    void CreateTable() override
    {
        const char* sql = "CREATE TABLE IF NOT EXISTS events ("
                          "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                          "event_data TEXT NOT NULL, "
                          "event_type TEXT NOT NULL, "
                          "timestamp DATETIME DEFAULT CURRENT_TIMESTAMP, "
                          "status TEXT DEFAULT 'pending'"
                          ");";
        char* errMsg = nullptr;
        int rc = sqlite3_exec(db, sql, nullptr, nullptr, &errMsg);
        if (rc != SQLITE_OK)
        {
            std::cerr << "Error creating table: " << errMsg << std::endl;
            sqlite3_free(errMsg);
        }
    }

    void InsertEvent(int id, const std::string& event_data, const std::string& event_type) override
    {
        const char* sql = "INSERT INTO events (event_data, event_type) VALUES (?, ?);";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, event_data.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, event_type.c_str(), -1, SQLITE_STATIC);
        sqlite3_step(stmt);
        sqlite3_finalize(stmt);
    }

    std::vector<Event> FetchPendingEvents(int limit) override
    {
        const char* sql = "SELECT id, event_data FROM events WHERE status = 'pending' LIMIT ?;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_int(stmt, 1, limit);

        std::vector<Event> events;
        while (sqlite3_step(stmt) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(stmt, 0);
            const unsigned char* event_data = sqlite3_column_text(stmt, 1);
            events.emplace_back(Event {id, std::string(reinterpret_cast<const char*>(event_data)), "", "pending"});
        }
        sqlite3_finalize(stmt);
        return events;
    }

    std::vector<Event> FetchAndMarkPendingEvents(int limit) override
    {
        const char* select_sql = "SELECT id, event_data FROM events WHERE status = 'pending' LIMIT ?;";
        const char* update_sql = "UPDATE events SET status = 'processing' WHERE id = ?;";

        sqlite3_stmt* select_stmt;
        sqlite3_stmt* update_stmt;
        sqlite3_prepare_v2(db, select_sql, -1, &select_stmt, nullptr);
        sqlite3_prepare_v2(db, update_sql, -1, &update_stmt, nullptr);
        sqlite3_bind_int(select_stmt, 1, limit);

        sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);

        std::vector<Event> events;
        while (sqlite3_step(select_stmt) == SQLITE_ROW)
        {
            int id = sqlite3_column_int(select_stmt, 0);
            const unsigned char* event_data = sqlite3_column_text(select_stmt, 1);
            events.emplace_back(Event {id, std::string(reinterpret_cast<const char*>(event_data)), "", "processing"});
            sqlite3_bind_int(update_stmt, 1, id);
            sqlite3_step(update_stmt);
            sqlite3_reset(update_stmt);
        }

        sqlite3_exec(db, "END TRANSACTION;", nullptr, nullptr, nullptr);
        sqlite3_finalize(select_stmt);
        sqlite3_finalize(update_stmt);
        return events;
    }

    void UpdateEventStatus(const std::vector<int>& event_ids, const std::string& status) override
    {
        const std::string sql = "UPDATE events SET status = '" + status + "' WHERE id = ?;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql.c_str(), -1, &stmt, nullptr);

        for (int id : event_ids)
        {
            sqlite3_bind_int(stmt, 1, id);
            sqlite3_step(stmt);
            sqlite3_reset(stmt);
        }
        sqlite3_finalize(stmt);
    }

    void DeleteEntriesWithStatus(const std::string& status) override
    {
        const char* sql = "DELETE FROM events WHERE status = ?;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, status.c_str(), -1, SQLITE_STATIC);

        sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
        sqlite3_step(stmt);
        sqlite3_exec(db, "END TRANSACTION;", nullptr, nullptr, nullptr);

        sqlite3_finalize(stmt);
    }

    void UpdateEntriesStatus(const std::string& from_status, const std::string& to_status) override
    {
        const char* sql = "UPDATE events SET status = ? WHERE status = ?;";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);
        sqlite3_bind_text(stmt, 1, to_status.c_str(), -1, SQLITE_STATIC);
        sqlite3_bind_text(stmt, 2, from_status.c_str(), -1, SQLITE_STATIC);

        sqlite3_exec(db, "BEGIN TRANSACTION;", nullptr, nullptr, nullptr);
        sqlite3_step(stmt);
        sqlite3_exec(db, "END TRANSACTION;", nullptr, nullptr, nullptr);

        sqlite3_finalize(stmt);
    }

    int GetPendingEventCount() override
    {
        const char* sql = "SELECT COUNT(*) FROM events WHERE status = 'pending';";
        sqlite3_stmt* stmt;
        sqlite3_prepare_v2(db, sql, -1, &stmt, nullptr);

        int count = 0;
        if (sqlite3_step(stmt) == SQLITE_ROW)
        {
            count = sqlite3_column_int(stmt, 0);
        }
        sqlite3_finalize(stmt);
        return count;
    }

    void InsertCommand(const std::string& command_data) override {}

    Command FetchPendingCommand()
    {
        Command command;
        return command;
    }

    void UpdateCommandStatus(int commandId) {}

private:
    sqlite3* db = nullptr;
};
