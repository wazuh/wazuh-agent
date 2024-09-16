#include <agent_info_persistance.hpp>

#include <SQLiteCpp/SQLiteCpp.h>
#include <logger.hpp>

AgentInfoPersistance::AgentInfoPersistance(const std::string& dbPath)
{
    try
    {
        m_db = std::make_unique<SQLite::Database>(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

        if (!AgentInfoTableExists())
        {
            CreateAgentInfoTable();
        }

        if (AgentInfoIsEmpty())
        {
            InsertDefaultAgentInfo();
        }
    }
    catch (const std::exception& e)
    {
        LogError("Can't open database: {}.", e.what());
        m_db.reset();
    }
}

AgentInfoPersistance::~AgentInfoPersistance() = default;

bool AgentInfoPersistance::AgentInfoTableExists() const
{
    try
    {
        SQLite::Statement query(*m_db, "SELECT name FROM sqlite_master WHERE type='table' AND name='agent_info';");
        return query.executeStep();
    }
    catch (const std::exception& e)
    {
        LogError("Failed to check if table exists: {}.", e.what());
        return false;
    }
}

bool AgentInfoPersistance::AgentInfoIsEmpty() const
{
    try
    {
        SQLite::Statement query(*m_db, "SELECT COUNT(*) FROM agent_info;");
        query.executeStep();
        const auto count = query.getColumn(0).getInt();
        return count == 0;
    }
    catch (const std::exception& e)
    {
        LogError("Error fetching: {}.", e.what());
    }

    return false;
}

void AgentInfoPersistance::CreateAgentInfoTable()
{
    try
    {
        m_db->exec("CREATE TABLE IF NOT EXISTS agent_info ("
                   "name TEXT, "
                   "key TEXT, "
                   "uuid TEXT"
                   ");");
    }
    catch (const std::exception& e)
    {
        LogError("Error creating table: {}.", e.what());
    }
}

void AgentInfoPersistance::InsertDefaultAgentInfo()
{
    try
    {
        SQLite::Statement query(*m_db, "SELECT COUNT(*) FROM agent_info;");
        query.executeStep();
        const auto count = query.getColumn(0).getInt();

        if (count == 0)
        {
            SQLite::Statement insert(*m_db, "INSERT INTO agent_info (name, key, uuid) VALUES (?, ?, ?);");
            insert.exec();
        }
    }
    catch (const std::exception& e)
    {
        LogError("Error inserting default agent info: {}.", e.what());
    }
}

void AgentInfoPersistance::SetAgentInfoValue(const std::string& column, const std::string& value)
{
    try
    {
        SQLite::Statement query(*m_db, "UPDATE agent_info SET " + column + " = ?;");
        query.bind(1, value);
        query.exec();
    }
    catch (const std::exception& e)
    {
        LogError("Error updating {}: {}.", column, e.what());
    }
}

std::string AgentInfoPersistance::GetAgentInfoValue(const std::string& column) const
{
    std::string value;
    try
    {
        SQLite::Statement query(*m_db, "SELECT " + column + " FROM agent_info LIMIT 1;");
        if (query.executeStep())
        {
            value = query.getColumn(0).getText();
        }
    }
    catch (const std::exception& e)
    {
        LogError("Error fetching {}: {}.", column, e.what());
    }
    return value;
}

std::string AgentInfoPersistance::GetName() const
{
    return GetAgentInfoValue("name");
}

std::string AgentInfoPersistance::GetKey() const
{
    return GetAgentInfoValue("key");
}

std::string AgentInfoPersistance::GetUUID() const
{
    return GetAgentInfoValue("uuid");
}

void AgentInfoPersistance::SetName(const std::string& name)
{
    SetAgentInfoValue("name", name);
}

void AgentInfoPersistance::SetKey(const std::string& key)
{
    SetAgentInfoValue("key", key);
}

void AgentInfoPersistance::SetUUID(const std::string& uuid)
{
    SetAgentInfoValue("uuid", uuid);
}

void AgentInfoPersistance::ResetToDefault()
{
    try
    {
        m_db->exec("DELETE FROM agent_info;");
        InsertDefaultAgentInfo();
    }
    catch (const std::exception& e)
    {
        LogError("Error resetting to default values: {}.", e.what());
    }
}
