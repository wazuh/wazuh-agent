#pragma once

#include <module_command/command_entry.hpp>
#include <sqlite_manager.hpp>

#include <memory>
#include <optional>
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
        module_command::Status StatusFromInt(const int i);

    public:
        CommandStore();

        bool Clear();
        int GetCount();
        bool StoreCommand(const module_command::CommandEntry& cmd);
        bool DeleteCommand(const std::string& id);
        std::optional<module_command::CommandEntry> GetCommand(const std::string& id);
        bool UpdateCommand(const module_command::CommandEntry& cmd);
    };
} // namespace command_store
