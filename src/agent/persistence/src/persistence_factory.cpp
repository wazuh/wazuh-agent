#include <persistence_factory.hpp>
#include <sqlite_manager.hpp>

#include <stdexcept>
#include <string>

std::unique_ptr<Persistence> PersistenceFactory::CreatePersistence(PersistenceType type, const std::string& dbName)
{
    if (type == PersistenceType::SQLITE3)
    {
        return std::make_unique<SQLiteManager>(dbName);
    }
    throw std::runtime_error("Unknown persistence type");
}
