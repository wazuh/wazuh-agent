#pragma once

#include <command.hpp>
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

        double GetCurrentTimestampAsReal();
        Status StatusFromInt(const int i);

    public:
        CommandStore();

        void Clear();
        int GetCount();
        void StoreCommand(const Command& cmd);
        void DeleteCommand(int id);
        Command GetCommand(int id);
    };
} // namespace command_store
