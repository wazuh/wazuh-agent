#include <command_store.hpp>

#include <logger.hpp>

#include <chrono>
#include <vector>

#include <nlohmann/json.hpp>

#include <column.hpp>
#include <persistence.hpp>
#include <persistence_factory.hpp>

using namespace column;

namespace
{
    // database
    const std::string COMMANDSTORE_DB_NAME = "command_store.db";

    // command_store table
    const std::string COMMAND_STORE_TABLE_NAME = "command_store";
    const std::string COMMAND_STORE_ID_COLUMN_NAME = "id";
    const std::string COMMAND_STORE_MODULE_COLUMN_NAME = "module";
    const std::string COMMAND_STORE_COMMAND_COLUMN_NAME = "command";
    const std::string COMMAND_STORE_PARAMETERS_COLUMN_NAME = "parameters";
    const std::string COMMAND_STORE_EXECUTION_MODE_COLUMN_NAME = "execution_mode";
    const std::string COMMAND_STORE_RESULT_COLUMN_NAME = "result";
    const std::string COMMAND_STORE_STATUS_COLUMN_NAME = "status";
    const std::string COMMAND_STORE_TIME_COLUMN_NAME = "time";
} // namespace

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

    module_command::CommandExecutionMode CommandStore::ExecutionModeFromInt(const int i)
    {
        if (i == 0)
        {
            return module_command::CommandExecutionMode::SYNC;
        }
        else
        {
            return module_command::CommandExecutionMode::ASYNC;
        }
    }

    CommandStore::CommandStore(const std::string& dbFolderPath, std::unique_ptr<Persistence> persistence)
    {
        const auto dbFilePath = dbFolderPath + "/" + COMMANDSTORE_DB_NAME;

        try
        {
            if (persistence)
            {
                m_dataBase = std::move(persistence);
            }
            else
            {
                m_dataBase =
                    PersistenceFactory::CreatePersistence(PersistenceFactory::PersistenceType::SQLITE3, dbFilePath);
            }

            if (!m_dataBase->TableExists(COMMAND_STORE_TABLE_NAME))
            {
                Keys columns;
                columns.emplace_back(COMMAND_STORE_ID_COLUMN_NAME, ColumnType::TEXT, NOT_NULL | PRIMARY_KEY);
                columns.emplace_back(COMMAND_STORE_MODULE_COLUMN_NAME, ColumnType::TEXT, NOT_NULL);
                columns.emplace_back(COMMAND_STORE_COMMAND_COLUMN_NAME, ColumnType::TEXT, NOT_NULL);
                columns.emplace_back(COMMAND_STORE_PARAMETERS_COLUMN_NAME, ColumnType::TEXT, NOT_NULL);
                columns.emplace_back(COMMAND_STORE_EXECUTION_MODE_COLUMN_NAME, ColumnType::INTEGER, NOT_NULL);
                columns.emplace_back(COMMAND_STORE_RESULT_COLUMN_NAME, ColumnType::TEXT, NOT_NULL);
                columns.emplace_back(COMMAND_STORE_STATUS_COLUMN_NAME, ColumnType::INTEGER, NOT_NULL);
                columns.emplace_back(COMMAND_STORE_TIME_COLUMN_NAME, ColumnType::REAL, NOT_NULL);

                try
                {
                    m_dataBase->CreateTable(COMMAND_STORE_TABLE_NAME, columns);
                }
                catch (std::exception& e)
                {
                    LogError("CreateTable operation failed: {}.", e.what());
                    throw;
                }
            }
        }

        catch (const std::exception&)
        {
            throw std::runtime_error(std::string("Cannot open database: " + dbFilePath));
        }
    }

    CommandStore::~CommandStore() = default;

    bool CommandStore::Clear()
    {
        try
        {
            m_dataBase->Remove(COMMAND_STORE_TABLE_NAME, {});
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
            count = m_dataBase->GetCount(COMMAND_STORE_TABLE_NAME);
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
        fields.emplace_back(COMMAND_STORE_ID_COLUMN_NAME, ColumnType::TEXT, cmd.Id);
        fields.emplace_back(COMMAND_STORE_MODULE_COLUMN_NAME, ColumnType::TEXT, cmd.Module);
        fields.emplace_back(COMMAND_STORE_COMMAND_COLUMN_NAME, ColumnType::TEXT, cmd.Command);
        fields.emplace_back(
            COMMAND_STORE_TIME_COLUMN_NAME, ColumnType::REAL, std::to_string(GetCurrentTimestampAsReal()));
        fields.emplace_back(COMMAND_STORE_PARAMETERS_COLUMN_NAME, ColumnType::TEXT, cmd.Parameters.dump());
        fields.emplace_back(COMMAND_STORE_EXECUTION_MODE_COLUMN_NAME,
                            ColumnType::INTEGER,
                            std::to_string(static_cast<int>(cmd.ExecutionMode)));
        fields.emplace_back(COMMAND_STORE_RESULT_COLUMN_NAME, ColumnType::TEXT, cmd.ExecutionResult.Message);
        fields.emplace_back(COMMAND_STORE_STATUS_COLUMN_NAME,
                            ColumnType::INTEGER,
                            std::to_string(static_cast<int>(cmd.ExecutionResult.ErrorCode)));

        try
        {
            m_dataBase->Insert(COMMAND_STORE_TABLE_NAME, fields);
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
        filters.emplace_back(COMMAND_STORE_ID_COLUMN_NAME, ColumnType::TEXT, id);

        try
        {
            m_dataBase->Remove(COMMAND_STORE_TABLE_NAME, filters);
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
        filters.emplace_back(COMMAND_STORE_ID_COLUMN_NAME, ColumnType::TEXT, id);

        try
        {
            auto cmdData = m_dataBase->Select(COMMAND_STORE_TABLE_NAME, {}, filters);

            if (cmdData.empty())
            {
                return std::nullopt;
            }

            module_command::CommandEntry cmd;
            for (const auto& col : cmdData[0])
            {
                if (col.Name == COMMAND_STORE_ID_COLUMN_NAME)
                {
                    cmd.Id = col.Value;
                }
                else if (col.Name == COMMAND_STORE_MODULE_COLUMN_NAME)
                {
                    cmd.Module = col.Value;
                }
                else if (col.Name == COMMAND_STORE_COMMAND_COLUMN_NAME)
                {
                    cmd.Command = col.Value;
                }
                else if (col.Name == COMMAND_STORE_PARAMETERS_COLUMN_NAME)
                {
                    cmd.Parameters = nlohmann::json::parse(col.Value);
                }
                else if (col.Name == COMMAND_STORE_EXECUTION_MODE_COLUMN_NAME)
                {
                    cmd.ExecutionMode = ExecutionModeFromInt(std::stoi(col.Value));
                }
                else if (col.Name == COMMAND_STORE_RESULT_COLUMN_NAME)
                {
                    cmd.ExecutionResult.Message = col.Value;
                }
                else if (col.Name == COMMAND_STORE_STATUS_COLUMN_NAME)
                {
                    cmd.ExecutionResult.ErrorCode = StatusFromInt(std::stoi(col.Value));
                }
                else if (col.Name == COMMAND_STORE_TIME_COLUMN_NAME)
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
        filters.emplace_back(
            COMMAND_STORE_STATUS_COLUMN_NAME, ColumnType::INTEGER, std::to_string(static_cast<int>(status)));

        try
        {
            auto cmdData = m_dataBase->Select(COMMAND_STORE_TABLE_NAME, {}, filters);

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
                    if (col.Name == COMMAND_STORE_ID_COLUMN_NAME)
                    {
                        cmd.Id = col.Value;
                    }
                    else if (col.Name == COMMAND_STORE_MODULE_COLUMN_NAME)
                    {
                        cmd.Module = col.Value;
                    }
                    else if (col.Name == COMMAND_STORE_COMMAND_COLUMN_NAME)
                    {
                        cmd.Command = col.Value;
                    }
                    else if (col.Name == COMMAND_STORE_PARAMETERS_COLUMN_NAME)
                    {
                        cmd.Parameters = nlohmann::json::parse(col.Value);
                    }
                    else if (col.Name == COMMAND_STORE_EXECUTION_MODE_COLUMN_NAME)
                    {
                        cmd.ExecutionMode = ExecutionModeFromInt(std::stoi(col.Value));
                    }
                    else if (col.Name == COMMAND_STORE_RESULT_COLUMN_NAME)
                    {
                        cmd.ExecutionResult.Message = col.Value;
                    }
                    else if (col.Name == COMMAND_STORE_STATUS_COLUMN_NAME)
                    {
                        cmd.ExecutionResult.ErrorCode = StatusFromInt(std::stoi(col.Value));
                    }
                    else if (col.Name == COMMAND_STORE_TIME_COLUMN_NAME)
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
            fields.emplace_back(COMMAND_STORE_MODULE_COLUMN_NAME, ColumnType::TEXT, cmd.Module);
        if (!cmd.Command.empty())
            fields.emplace_back(COMMAND_STORE_COMMAND_COLUMN_NAME, ColumnType::TEXT, cmd.Command);
        if (!cmd.Parameters.empty())
            fields.emplace_back(COMMAND_STORE_PARAMETERS_COLUMN_NAME, ColumnType::TEXT, cmd.Parameters.dump());
        if (!cmd.ExecutionResult.Message.empty())
            fields.emplace_back(COMMAND_STORE_RESULT_COLUMN_NAME, ColumnType::TEXT, cmd.ExecutionResult.Message);
        if (cmd.ExecutionResult.ErrorCode != module_command::Status::UNKNOWN)
            fields.emplace_back(COMMAND_STORE_STATUS_COLUMN_NAME,
                                ColumnType::INTEGER,
                                std::to_string(static_cast<int>(cmd.ExecutionResult.ErrorCode)));

        Criteria filters;
        filters.emplace_back(COMMAND_STORE_ID_COLUMN_NAME, ColumnType::TEXT, cmd.Id);

        try
        {
            m_dataBase->Update(COMMAND_STORE_TABLE_NAME, fields, filters);
        }
        catch (const std::exception& e)
        {
            LogError("UpdateCommand operation failed: {}.", e.what());
            return false;
        }
        return true;
    }
} // namespace command_store
