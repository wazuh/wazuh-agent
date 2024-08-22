#include <persistence_factory.hpp>
#include <sqlitestorage.hpp>

#include <stdexcept>
#include <string>

std::unique_ptr<Persistence> PersistenceFactory::createPersistence(PersistenceType type,
                                                                   const std::vector<std::any>& args)
{
    if (type == PersistenceType::SQLITE3)
    {
        if (args.size() != 2 || !std::any_cast<std::string>(&args[0]) ||
            !std::any_cast<std::vector<std::string>>(&args[1]))
        {
            throw std::invalid_argument("SQLite3 requires db name and table names as arguments");
        }
        return std::make_unique<SQLiteStorage>(std::any_cast<std::string>(args[0]),
                                               std::any_cast<std::vector<std::string>>(args[1]));
    }
    throw std::runtime_error("Unknown persistence type");
}
