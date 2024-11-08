#include <agent_info_persistance.hpp>

#include <logger.hpp>

using namespace sqlite_manager;

namespace
{
    const std::string AGENT_INFO_TABLE_NAME = "agent_info";
    const std::string AGENT_GROUP_TABLE_NAME = "agent_group";
} // namespace

AgentInfoPersistance::AgentInfoPersistance(const std::string& dbPath)
{
    try
    {
        m_db = std::make_unique<SQLiteManager>(dbPath);

        if (!m_db->TableExists(AGENT_INFO_TABLE_NAME))
        {
            CreateAgentInfoTable();
        }

        if (!m_db->TableExists(AGENT_GROUP_TABLE_NAME))
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

bool AgentInfoPersistance::AgentInfoIsEmpty() const
{
    try
    {
        return m_db->GetCount(AGENT_INFO_TABLE_NAME) == 0;
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
        const std::vector<Column> columns = {Column("key", ColumnType::TEXT, true, false),
                                             Column("uuid", ColumnType::TEXT, true, false, true)};

        m_db->CreateTable(AGENT_INFO_TABLE_NAME, columns);
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
        const std::vector<Column> columns = {Column("id", ColumnType::INTEGER, true, true, true),
                                             Column("name", ColumnType::TEXT, true, false)};

        m_db->CreateTable(AGENT_GROUP_TABLE_NAME, columns);
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
        const auto count = m_db->GetCount(AGENT_INFO_TABLE_NAME);

        if (count == 0)
        {
            const std::vector<Column> columns = {Column("key", ColumnType::TEXT, ""),
                                                 Column("uuid", ColumnType::TEXT, "")};

            m_db->Insert(AGENT_INFO_TABLE_NAME, columns);
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
        const std::vector<Column> columns = {Column(column, ColumnType::TEXT, value)};
        m_db->Update(AGENT_INFO_TABLE_NAME, columns);
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
        const std::vector<Column> columns = {Column(column, ColumnType::TEXT, "")};
        const std::vector<Row> results = m_db->Select(AGENT_INFO_TABLE_NAME, columns);

        if (!results.empty() && !results[0].empty())
        {
            value = results[0][0].Value;
        }
    }
    catch (const std::exception& e)
    {
        LogError("Error fetching {}: {}.", column, e.what());
    }

    return value;
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
        const std::vector<Column> columns = {Column("name", ColumnType::TEXT, "")};
        const std::vector<Row> results = m_db->Select(AGENT_GROUP_TABLE_NAME, columns);

        for (const auto& row : results)
        {
            if (!row.empty())
            {
                groupList.push_back(row[0].Value);
            }
        }
    }
    catch (const std::exception& e)
    {
        LogError("Error getting agent group list: {}.", e.what());
    }

    return groupList;
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
    auto transaction = m_db->BeginTransaction();

    try
    {
        m_db->Remove(AGENT_GROUP_TABLE_NAME);

        for (const auto& group : groupList)
        {
            const std::vector<Column> columns = {Column("name", ColumnType::TEXT, group)};
            m_db->Insert(AGENT_GROUP_TABLE_NAME, columns);
        }

        m_db->CommitTransaction(transaction);
    }
    catch (const std::exception& e)
    {
        LogError("Error inserting group: {}.", e.what());
        m_db->RollbackTransaction(transaction);
    }
}

void AgentInfoPersistance::ResetToDefault()
{
    try
    {
        m_db->Remove(AGENT_INFO_TABLE_NAME);
        m_db->Remove(AGENT_GROUP_TABLE_NAME);
        InsertDefaultAgentInfo();
    }
    catch (const std::exception& e)
    {
        LogError("Error resetting to default values: {}.", e.what());
    }
}
