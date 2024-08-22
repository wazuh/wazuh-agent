#include <persistence_factory.hpp>
#include <sqlitestorage.hpp>

#include <stdexcept>
#include <string>

std::unique_ptr<IPersistence> PersistenceFactory::createPersistence(PersistenceType type, const std::string& dbName)
{
    if (type == PersistenceType::SQLITE3)
    {
        if (dbName.size() == 0)
        {
            throw std::invalid_argument("SQLite3 requires db name as argument");
        }
        return std::make_unique<SQLiteStorage>(dbName);
    }
    throw std::runtime_error("Unknown persistence type");
}