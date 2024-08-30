#include <sqlite_manager.hpp>

namespace sqlite_manager
{
    SQLiteManager::SQLiteManager(const std::string& dbName)
        : m_dbName(dbName)
        , m_db(std::make_unique<SQLite::Database>(dbName, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE))
    {
        m_db->exec("PRAGMA journal_mode=WAL;");
    }
} // namespace sqlite_manager
