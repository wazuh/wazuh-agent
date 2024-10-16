#include <agent_info_persistance.hpp>

#include <SQLiteCpp/SQLiteCpp.h>
#include <logger.hpp>

namespace
{
    const std::string AGENT_INFO_TABLE_NAME = "agent_info";
    const std::string AGENT_GROUP_TABLE_NAME = "agent_group";
} // namespace

AgentInfoPersistance::AgentInfoPersistance(const std::string& dbPath)
{
    try
    {
        m_db = std::make_unique<SQLite::Database>(dbPath, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);

        if (!TableExists(AGENT_INFO_TABLE_NAME))
        {
            CreateAgentInfoTable();
        }

        if (!TableExists(AGENT_GROUP_TABLE_NAME))
        {
            CreateAgentGroupTable();
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

bool AgentInfoPersistance::TableExists(const std::string& table) const
{
    try
    {
        SQLite::Statement query(*m_db, "SELECT name FROM sqlite_master WHERE type='table' AND name='" + table + "';");
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

void AgentInfoPersistance::CreateAgentGroupTable()
{
    try
    {
        m_db->exec("CREATE TABLE IF NOT EXISTS " + AGENT_GROUP_TABLE_NAME +
                   " ("
                   "id INTEGER PRIMARY KEY AUTOINCREMENT,"
                   "name TEXT NOT NULL UNIQUE"
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

std::vector<std::string> AgentInfoPersistance::GetGroups() const
{
    std::vector<std::string> groupList;

    try
    {
        SQLite::Statement query(*m_db, "SELECT name FROM " + AGENT_GROUP_TABLE_NAME + " ORDER BY id ASC;");

        while (query.executeStep())
        {
            groupList.push_back(query.getColumn(0).getString());
        }
    }
    catch (const std::exception& e)
    {
        LogError("Error getting agent group list: {}.", e.what());
    }

    return groupList;
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

void AgentInfoPersistance::SetGroups(const std::vector<std::string>& groupList)
{
    try
    {
        SQLite::Transaction transaction(*m_db);

        m_db->exec("DELETE FROM " + AGENT_GROUP_TABLE_NAME + ";");

        SQLite::Statement query(*m_db, "INSERT INTO " + AGENT_GROUP_TABLE_NAME + " (name) VALUES (?);");

        for (const std::string& group : groupList)
        {
            query.bind(1, group);
            query.exec();
            query.reset();
        }

        transaction.commit();
    }
    catch (const std::exception& e)
    {
        LogError("Error inserting group: {}.", e.what());
    }
}

void AgentInfoPersistance::ResetToDefault()
{
    try
    {
        m_db->exec("DELETE FROM " + AGENT_INFO_TABLE_NAME + ";");
        m_db->exec("DELETE FROM " + AGENT_GROUP_TABLE_NAME + ";");
        InsertDefaultAgentInfo();
    }
    catch (const std::exception& e)
    {
        LogError("Error resetting to default values: {}.", e.what());
    }
}
