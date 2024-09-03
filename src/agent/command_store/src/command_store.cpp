#include <command_store.hpp>

#include <iostream>

namespace command_store
{
    CommandStore::CommandStore()
        : m_dataBase(std::make_unique<sqlite_manager::SQLiteManager>(COMMANDSTORE_DEFAULT_DB_PATH))
    {
        sqlite_manager::Column colId {"Id", sqlite_manager::ColumnType::INTEGER, true, false, true};
        sqlite_manager::Column colModule {"Module", sqlite_manager::ColumnType::TEXT, true, false, false};
        sqlite_manager::Column colCommand {"Command", sqlite_manager::ColumnType::TEXT, true, false, false};
        sqlite_manager::Column colTime {"Time", sqlite_manager::ColumnType::REAL, true, false, false};
        m_dataBase->CreateTable(COMMANDSTORE_TABLE_NAME, {colId, colModule, colCommand, colTime});
    }

    void CommandStore::StoreCommand(const std::string& command)
    {
        std::cout << "Store command: " << command << "\n";
    }

    void CommandStore::DeleteCommand(int id)
    {
        std::cout << "Deleting command " << id << "\n";
    }

} // namespace command_store
