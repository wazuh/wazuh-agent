#include <command_store.hpp>

#include <logger.hpp>

#include <chrono>

namespace command_store
{
    constexpr double MILLISECS_IN_A_SEC = 1000.0;

    Status CommandStore::StatusFromInt(const int i)
    {
        switch (i)
        {
            case 0: return Status::SUCCESS; break;
            case 1: return Status::FAILURE; break;
            case 2: return Status::IN_PROGRESS; break;
            case 3: return Status::TIMEOUT; break;
            default: return Status::UNKNOWN; break;
        }
    }

    CommandStore::CommandStore()
        : m_dataBase(std::make_unique<sqlite_manager::SQLiteManager>(COMMANDSTORE_DEFAULT_DB_PATH))
    {
        sqlite_manager::Column colId {"id", sqlite_manager::ColumnType::TEXT, true, false, true};
        sqlite_manager::Column colModule {"module", sqlite_manager::ColumnType::TEXT, true, false, false};
        sqlite_manager::Column colCommand {"command", sqlite_manager::ColumnType::TEXT, true, false, false};
        sqlite_manager::Column colParameter {"parameters", sqlite_manager::ColumnType::TEXT, true, false, false};
        sqlite_manager::Column colResult {"result", sqlite_manager::ColumnType::TEXT, true, false, false};
        sqlite_manager::Column colStatus {"status", sqlite_manager::ColumnType::INTEGER, true, false, false};
        sqlite_manager::Column colTime {"time", sqlite_manager::ColumnType::REAL, true, false, false};

        try
        {
            m_dataBase->CreateTable(COMMANDSTORE_TABLE_NAME,
                                    {colId, colModule, colCommand, colParameter, colResult, colStatus, colTime});
        }
        catch (std::exception& e)
        {
            LogError("CreateTable operation failed: {}.", e.what());
        }
    }

    bool CommandStore::Clear()
    {
        try
        {
            m_dataBase->Remove(COMMANDSTORE_TABLE_NAME, {});
        }
        catch (const std::exception& e)
        {
            LogError("Clear operation failed: {}.", e.what());
            return false;
        }
        return true;
    }

    int CommandStore::GetCount()
    {
        int count = 0;
        try
        {
            count = m_dataBase->GetCount(COMMANDSTORE_TABLE_NAME);
        }
        catch (const std::exception& e)
        {
            LogError("GetCount operation failed: {}.", e.what());
        }

        return count;
    }

    double CommandStore::GetCurrentTimestampAsReal()
    {
        auto now = std::chrono::system_clock::now();
        auto now_ms = std::chrono::time_point_cast<std::chrono::milliseconds>(now);
        auto epoch = now_ms.time_since_epoch();

        return static_cast<double>(std::chrono::duration_cast<std::chrono::milliseconds>(epoch).count()) /
               MILLISECS_IN_A_SEC;
    }

    bool CommandStore::StoreCommand(const CommandEntry& cmd)
    {
        std::vector<sqlite_manager::Column> fields;
        fields.emplace_back("id", sqlite_manager::ColumnType::TEXT, cmd.Id);
        fields.emplace_back("module", sqlite_manager::ColumnType::TEXT, cmd.Module);
        fields.emplace_back("command", sqlite_manager::ColumnType::TEXT, cmd.Command);
        fields.emplace_back("time", sqlite_manager::ColumnType::REAL, std::to_string(GetCurrentTimestampAsReal()));
        fields.emplace_back("parameters", sqlite_manager::ColumnType::TEXT, cmd.Parameters);
        fields.emplace_back("result", sqlite_manager::ColumnType::TEXT, cmd.Result);
        fields.emplace_back(
            "status", sqlite_manager::ColumnType::INTEGER, std::to_string(static_cast<int>(cmd.CurrentStatus)));
        try
        {
            m_dataBase->Insert(COMMANDSTORE_TABLE_NAME, fields);
        }
        catch (const std::exception& e)
        {
            LogError("StoreCommand operation failed: {}.", e.what());
            return false;
        }
        return true;
    }

    bool CommandStore::DeleteCommand(const std::string& id)
    {
        std::vector<sqlite_manager::Column> fields;
        fields.emplace_back("id", sqlite_manager::ColumnType::TEXT, id);
        try
        {
            m_dataBase->Remove(COMMANDSTORE_TABLE_NAME, fields);
        }
        catch (const std::exception& e)
        {
            LogError("DeleteCommand operation failed: {}.", e.what());
            return false;
        }
        return true;
    }

    std::optional<CommandEntry> CommandStore::GetCommand(const std::string& id)
    {
        try
        {
            auto cmdData = m_dataBase->Select(
                COMMANDSTORE_TABLE_NAME, {}, {sqlite_manager::Column("id", sqlite_manager::ColumnType::TEXT, id)});

            if (cmdData.empty())
            {
                return std::nullopt;
            }

            CommandEntry cmd;
            for (const sqlite_manager::Column& col : cmdData[0])
            {
                if (col.Name == "id")
                {
                    cmd.Id = col.Value;
                }
                else if (col.Name == "module")
                {
                    cmd.Module = col.Value;
                }
                else if (col.Name == "command")
                {
                    cmd.Command = col.Value;
                }
                else if (col.Name == "parameters")
                {
                    cmd.Parameters = col.Value;
                }
                else if (col.Name == "result")
                {
                    cmd.Result = col.Value;
                }
                else if (col.Name == "status")
                {
                    cmd.CurrentStatus = StatusFromInt(std::stoi(col.Value));
                }
                else if (col.Name == "time")
                {
                    cmd.Time = std::stod(col.Value);
                }
            }
            return cmd;
        }
        catch (const std::exception& e)
        {
            LogError("Select operation failed: {}.", e.what());
            return std::nullopt;
        }
    }

    bool CommandStore::UpdateCommand(const CommandEntry& cmd)
    {
        std::vector<sqlite_manager::Column> fields;
        if (!cmd.Module.empty())
            fields.emplace_back("module", sqlite_manager::ColumnType::TEXT, cmd.Module);
        if (!cmd.Command.empty())
            fields.emplace_back("command", sqlite_manager::ColumnType::TEXT, cmd.Command);
        if (!cmd.Parameters.empty())
            fields.emplace_back("parameters", sqlite_manager::ColumnType::TEXT, cmd.Parameters);
        if (!cmd.Result.empty())
            fields.emplace_back("result", sqlite_manager::ColumnType::TEXT, cmd.Result);
        if (cmd.CurrentStatus != Status::UNKNOWN)
            fields.emplace_back(
                "status", sqlite_manager::ColumnType::INTEGER, std::to_string(static_cast<int>(cmd.CurrentStatus)));

        sqlite_manager::Column condition("id", sqlite_manager::ColumnType::TEXT, cmd.Id);
        try
        {
            m_dataBase->Update(COMMANDSTORE_TABLE_NAME, fields, {condition});
        }
        catch (const std::exception& e)
        {
            LogError("UpdateCommand operation failed: {}.", e.what());
            return false;
        }
        return true;
    }
} // namespace command_store
