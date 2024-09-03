#pragma once

#include <sqlite_manager.hpp>

#include <memory>
#include <string>

namespace command_store
{
    const std::string COMMANDSTORE_DEFAULT_DB_PATH = "command_store.db";
    const std::string COMMANDSTORE_TABLE_NAME = "COMMAND";

    class CommandStore
    {
    private:
        std::unique_ptr<sqlite_manager::SQLiteManager> m_dataBase;

    public:
        CommandStore();

        void StoreCommand(const std::string& command);
        void DeleteCommand(int id);
    };
} // namespace command_store
