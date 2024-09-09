#include <command_store.hpp>

#include <chrono>
#include <iostream>

namespace command_store
{
    constexpr double MILLISECS_IN_A_SEC = 1000.0;

    Status CommandStore::StatusFromInt(const int i)
    {
        switch (i)
        {
            case 0: return Status::SUCCESS; break;
            case 1: return Status::ERROR; break;
            case 2: return Status::IN_PROGRESS; break;
            case 3: return Status::TIMEOUT; break;
            default: return Status::UNKNOWN; break;
        }
    }

    CommandStore::CommandStore()
        : m_dataBase(std::make_unique<sqlite_manager::SQLiteManager>(COMMANDSTORE_DEFAULT_DB_PATH))
    {
        sqlite_manager::Column colId {"id", sqlite_manager::ColumnType::INTEGER, true, false, true};
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
            std::cerr << "CreateTable operation failed. " << e.what() << "\n";
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
            std::cerr << "Clear operation failed. " << e.what() << "\n";
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
            std::cerr << "GetCount operation failed. " << e.what() << "\n";
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

    bool CommandStore::StoreCommand(const Command& cmd)
    {
        std::vector<sqlite_manager::Column> fields;
        fields.emplace_back("id", sqlite_manager::ColumnType::INTEGER, std::to_string(cmd.m_id));
        fields.emplace_back("module", sqlite_manager::ColumnType::TEXT, cmd.m_module);
        fields.emplace_back("command", sqlite_manager::ColumnType::TEXT, cmd.m_command);
        fields.emplace_back("time", sqlite_manager::ColumnType::REAL, std::to_string(GetCurrentTimestampAsReal()));
        fields.emplace_back("parameters", sqlite_manager::ColumnType::TEXT, cmd.m_parameters);
        fields.emplace_back("result", sqlite_manager::ColumnType::TEXT, cmd.m_result);
        fields.emplace_back(
            "status", sqlite_manager::ColumnType::INTEGER, std::to_string(static_cast<int>(cmd.m_status)));
        try
        {
            m_dataBase->Insert(COMMANDSTORE_TABLE_NAME, fields);
        }
        catch (const std::exception& e)
        {
            std::cerr << "StoreCommand operation failed. " << e.what() << "\n";
            return false;
        }
        return true;
    }

    bool CommandStore::DeleteCommand(int id)
    {
        std::vector<sqlite_manager::Column> fields;
        fields.emplace_back("id", sqlite_manager::ColumnType::INTEGER, std::to_string(id));
        try
        {
            m_dataBase->Remove(COMMANDSTORE_TABLE_NAME, fields);
        }
        catch (const std::exception& e)
        {
            std::cerr << "DeleteCommand operation failed. " << e.what() << "\n";
            return false;
        }
        return true;
    }

    std::optional<Command> CommandStore::GetCommand(int id)
    {
        try
        {
            auto cmdData = m_dataBase->Select(
                COMMANDSTORE_TABLE_NAME,
                {},
                {sqlite_manager::Column("id", sqlite_manager::ColumnType::INTEGER, std::to_string(id))});

            if (cmdData.empty())
            {
                return std::nullopt;
            }

            Command cmd;
            for (const sqlite_manager::Column& col : cmdData[0])
            {
                if (col.m_name == "id")
                {
                    cmd.m_id = std::stoi(col.m_value);
                }
                else if (col.m_name == "module")
                {
                    cmd.m_module = col.m_value;
                }
                else if (col.m_name == "command")
                {
                    cmd.m_command = col.m_value;
                }
                else if (col.m_name == "parameters")
                {
                    cmd.m_parameters = col.m_value;
                }
                else if (col.m_name == "result")
                {
                    cmd.m_result = col.m_value;
                }
                else if (col.m_name == "status")
                {
                    cmd.m_status = StatusFromInt(std::stoi(col.m_value));
                }
                else if (col.m_name == "time")
                {
                    cmd.m_time = std::stod(col.m_value);
                }
            }
            return cmd;
        }
        catch (const std::exception& e)
        {
            std::cerr << "Select operation failed. " << e.what() << "\n";
            return std::nullopt;
        }
    }

    bool CommandStore::UpdateCommand(const Command& cmd)
    {
        std::vector<sqlite_manager::Column> fields;
        if (!cmd.m_module.empty())
            fields.emplace_back("module", sqlite_manager::ColumnType::TEXT, cmd.m_module);
        if (!cmd.m_command.empty())
            fields.emplace_back("command", sqlite_manager::ColumnType::TEXT, cmd.m_command);
        if (!cmd.m_parameters.empty())
            fields.emplace_back("parameters", sqlite_manager::ColumnType::TEXT, cmd.m_parameters);
        if (!cmd.m_result.empty())
            fields.emplace_back("result", sqlite_manager::ColumnType::TEXT, cmd.m_result);
        if (cmd.m_status != Status::UNKNOWN)
            fields.emplace_back(
                "status", sqlite_manager::ColumnType::INTEGER, std::to_string(static_cast<int>(cmd.m_status)));

        sqlite_manager::Column condition("id", sqlite_manager::ColumnType::INTEGER, std::to_string(cmd.m_id));
        try
        {
            m_dataBase->Update(COMMANDSTORE_TABLE_NAME, fields, {condition});
        }
        catch (const std::exception& e)
        {
            std::cerr << "UpdateCommand operation failed. " << e.what() << "\n";
            return false;
        }
        return true;
    }
} // namespace command_store
