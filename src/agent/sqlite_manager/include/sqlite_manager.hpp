#pragma once

#include <SQLiteCpp/SQLiteCpp.h>

#include <memory>
#include <string>

namespace sqlite_manager
{
    class SQLiteManager
    {
    public:
        SQLiteManager(const std::string& dbName);

    private:
        /**
         * @brief The name of the SQLite database file.
         */
        const std::string m_dbName;

        /**
         * @brief Pointer to the SQLite database connection.
         */
        std::unique_ptr<SQLite::Database> m_db;
    };
} // namespace sqlite_manager
