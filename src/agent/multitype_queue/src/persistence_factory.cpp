#include <persistence_factory.hpp>
#include <sqlitestorage.hpp>

#include <stdexcept>
#include <string>

std::unique_ptr<IPersistence> PersistenceFactory::createPersistence(PersistenceType type,
                                                                    const std::string& dbName,
                                                                    const std::string_view& createTableQuery,
                                                                    const std::vector<std::string>& args)
{
    if (type == PersistenceType::SQLITE3)
    {
        if (dbName.size() == 0 || createTableQuery.size() == 0 || args.size() == 0)
        {
            throw std::invalid_argument("SQLite3 requires db name, table creation query and table names as arguments");
        }
        return std::make_unique<SQLiteStorage>(dbName, createTableQuery, args);
    }
    throw std::runtime_error("Unknown persistence type");
}
