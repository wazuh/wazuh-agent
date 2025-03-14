#pragma once

#include "commonDefs.h"
#include "db_exception.h"
#include "sqlite/sqlite_dbengine.h"
#include "sqliteWrapperFactory.hpp"
#include <iostream>

namespace DbSync
{
    /// @brief Factory class for creating DbEngines
    class FactoryDbEngine
    {
    public:
        /// @brief Creates a new DbEngine instance.
        /// @param dbType Database type to be used (currently only supported SQLITE3)
        /// @param path Path where the local database will be created.
        /// @param sqlStatement SQL sentence to create tables in a SQL engine.
        /// @param dbManagement Database management type to be used at startup.
        /// @param upgradeStatements SQL sentences to be executed when upgrading the database.
        /// @return Handle instance to be used for common sql operations (cannot be used by more than 1 thread).
        static std::unique_ptr<IDbEngine> create(const DbEngineType dbType,
                                                 const std::string& path,
                                                 const std::string& sqlStatement,
                                                 const DbManagement dbManagement,
                                                 const std::vector<std::string>& upgradeStatements)
        {
            if (SQLITE3 == dbType)
            {
                return std::make_unique<SQLiteDBEngine>(std::make_shared<SQLiteLegacy::SQLiteFactory>(),
                                                        path,
                                                        sqlStatement,
                                                        dbManagement,
                                                        upgradeStatements);
            }

            throw dbsync_error {FACTORY_INSTANTATION};
        }
    };
} // namespace DbSync
