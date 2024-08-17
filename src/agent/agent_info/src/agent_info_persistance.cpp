#include <agent_info_persistance.hpp>

#include <SQLiteCpp/SQLiteCpp.h>

#include <iostream>

AgentInfoPersistance::AgentInfoPersistance(const std::string& db_path)
{
    try
    {
        m_db = std::make_unique<SQLite::Database>(db_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

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
        std::cerr << "Can't open database: " << e.what() << std::endl;
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
        std::cerr << "Failed to check if table exists: " << e.what() << std::endl;
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
        std::cerr << "Error fetching: " << e.what() << std::endl;
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
        std::cerr << "Error creating table: " << e.what() << std::endl;
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
        std::cerr << "Error inserting default agent info: " << e.what() << std::endl;
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
        std::cerr << "Error updating " << column << ": " << e.what() << std::endl;
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
        std::cerr << "Error fetching " << column << ": " << e.what() << std::endl;
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
        std::cerr << "Error resetting to default values: " << e.what() << std::endl;
    }
}
