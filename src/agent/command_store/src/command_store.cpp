#include <command_store.hpp>

#include <logger.hpp>

#include <chrono>
#include <vector>

#include <nlohmann/json.hpp>

using namespace sqlite_manager;

namespace command_store
{
    constexpr double MILLISECS_IN_A_SEC = 1000.0;

    module_command::Status CommandStore::StatusFromInt(const int i)
    {
        switch (i)
        {
            case 0: return module_command::Status::SUCCESS; break;
            case 1: return module_command::Status::FAILURE; break;
            case 2: return module_command::Status::IN_PROGRESS; break;
            case 3: return module_command::Status::TIMEOUT; break;
            default: return module_command::Status::UNKNOWN; break;
        }
    }

    CommandStore::CommandStore(const std::string& dbFolderPath)
    {
        const auto dbFilePath = dbFolderPath + "/" + COMMANDSTORE_DB_NAME;

        try
        {
            m_dataBase = std::make_unique<SQLiteManager>(dbFilePath);

            if (!m_dataBase->TableExists(COMMANDSTORE_TABLE_NAME))
            {
                std::vector<ColumnKey> columns;
                columns.emplace_back("id", ColumnType::TEXT, true, false, true);
                columns.emplace_back("module", ColumnType::TEXT, true, false, false);
                columns.emplace_back("command", ColumnType::TEXT, true, false, false);
                columns.emplace_back("parameters", ColumnType::TEXT, true, false, false);
                columns.emplace_back("result", ColumnType::TEXT, true, false, false);
                columns.emplace_back("status", ColumnType::INTEGER, true, false, false);
                columns.emplace_back("time", ColumnType::REAL, true, false, false);

                try
                {
                    m_dataBase->CreateTable(COMMANDSTORE_TABLE_NAME, columns);
                }
                catch (std::exception& e)
                {
                    LogError("CreateTable operation failed: {}.", e.what());
                }
            }
        }

        catch (const std::exception&)
        {
            throw std::runtime_error(std::string("Cannot open database: " + dbFilePath));
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

    bool CommandStore::StoreCommand(const module_command::CommandEntry& cmd)
    {
        Row fields;
        fields.emplace_back("id", ColumnType::TEXT, cmd.Id);
        fields.emplace_back("module", ColumnType::TEXT, cmd.Module);
        fields.emplace_back("command", ColumnType::TEXT, cmd.Command);
        fields.emplace_back("time", ColumnType::REAL, std::to_string(GetCurrentTimestampAsReal()));
        fields.emplace_back("parameters", ColumnType::TEXT, cmd.Parameters.dump());
        fields.emplace_back("result", ColumnType::TEXT, cmd.ExecutionResult.Message);
        fields.emplace_back(
            "status", ColumnType::INTEGER, std::to_string(static_cast<int>(cmd.ExecutionResult.ErrorCode)));

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
        Criteria filters;
        filters.emplace_back("id", ColumnType::TEXT, id);

        try
        {
            m_dataBase->Remove(COMMANDSTORE_TABLE_NAME, filters);
        }
        catch (const std::exception& e)
        {
            LogError("DeleteCommand operation failed: {}.", e.what());
            return false;
        }
        return true;
    }

    std::optional<module_command::CommandEntry> CommandStore::GetCommand(const std::string& id)
    {
        Criteria filters;
        filters.emplace_back("id", ColumnType::TEXT, id);

        try
        {
            auto cmdData = m_dataBase->Select(COMMANDSTORE_TABLE_NAME, {}, filters);

            if (cmdData.empty())
            {
                return std::nullopt;
            }

            module_command::CommandEntry cmd;
            for (const auto& col : cmdData[0])
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
                    cmd.Parameters = nlohmann::json::parse(col.Value);
                }
                else if (col.Name == "result")
                {
                    cmd.ExecutionResult.Message = col.Value;
                }
                else if (col.Name == "status")
                {
                    cmd.ExecutionResult.ErrorCode = StatusFromInt(std::stoi(col.Value));
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

    std::optional<std::vector<module_command::CommandEntry>>
    CommandStore::GetCommandByStatus(const module_command::Status& status)
    {
        Criteria filters;
        filters.emplace_back("status", ColumnType::INTEGER, std::to_string(static_cast<int>(status)));

        try
        {
            auto cmdData = m_dataBase->Select(COMMANDSTORE_TABLE_NAME, {}, filters);

            if (cmdData.empty())
            {
                return std::nullopt;
            }

            std::vector<module_command::CommandEntry> commands;

            for (const auto& row : cmdData)
            {
                module_command::CommandEntry cmd;

                for (const auto& col : row)
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
                        cmd.Parameters = nlohmann::json::parse(col.Value);
                    }
                    else if (col.Name == "result")
                    {
                        cmd.ExecutionResult.Message = col.Value;
                    }
                    else if (col.Name == "status")
                    {
                        cmd.ExecutionResult.ErrorCode = StatusFromInt(std::stoi(col.Value));
                    }
                    else if (col.Name == "time")
                    {
                        cmd.Time = std::stod(col.Value);
                    }
                }

                commands.push_back(cmd);
            }
            return commands;
        }
        catch (const std::exception& e)
        {
            LogError("Select operation failed: {}.", e.what());
            return std::nullopt;
        }
    }

    bool CommandStore::UpdateCommand(const module_command::CommandEntry& cmd)
    {
        Row fields;
        if (!cmd.Module.empty())
            fields.emplace_back("module", ColumnType::TEXT, cmd.Module);
        if (!cmd.Command.empty())
            fields.emplace_back("command", ColumnType::TEXT, cmd.Command);
        if (!cmd.Parameters.empty())
            fields.emplace_back("parameters", ColumnType::TEXT, cmd.Parameters.dump());
        if (!cmd.ExecutionResult.Message.empty())
            fields.emplace_back("result", ColumnType::TEXT, cmd.ExecutionResult.Message);
        if (cmd.ExecutionResult.ErrorCode != module_command::Status::UNKNOWN)
            fields.emplace_back(
                "status", ColumnType::INTEGER, std::to_string(static_cast<int>(cmd.ExecutionResult.ErrorCode)));

        Criteria filters;
        filters.emplace_back("id", ColumnType::TEXT, cmd.Id);

        try
        {
            m_dataBase->Update(COMMANDSTORE_TABLE_NAME, fields, filters);
        }
        catch (const std::exception& e)
        {
            LogError("UpdateCommand operation failed: {}.", e.what());
            return false;
        }
        return true;
    }
} // namespace command_store
