#include <agent_info_persistance.hpp>

#include <logger.hpp>

using namespace sqlite_manager;

namespace
{
    const std::string AGENT_INFO_TABLE_NAME = "agent_info";
    const std::string AGENT_GROUP_TABLE_NAME = "agent_group";
    const std::string AGENT_INFO_DB_NAME = "agent_info.db";
} // namespace

AgentInfoPersistance::AgentInfoPersistance(const std::string& dbFolderPath)
{
    const auto dbFilePath = dbFolderPath + "/" + AGENT_INFO_DB_NAME;

    try
    {
        m_db = std::make_unique<SQLiteManager>(dbFilePath);

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
    catch (const std::exception&)
    {
        throw std::runtime_error(std::string("Cannot open database: " + dbFilePath));
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
        const std::vector<ColumnKey> columns = {ColumnKey("name", ColumnType::TEXT, true, false),
                                                ColumnKey("key", ColumnType::TEXT, true, false),
                                                ColumnKey("uuid", ColumnType::TEXT, true, false, true)};

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
        const std::vector<ColumnKey> columns = {ColumnKey("id", ColumnType::INTEGER, true, true, true),
                                                ColumnKey("name", ColumnType::TEXT, true, false)};

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
            const Row columns = {ColumnValue("name", ColumnType::TEXT, ""),
                                 ColumnValue("key", ColumnType::TEXT, ""),
                                 ColumnValue("uuid", ColumnType::TEXT, "")};

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
        const Row columns = {ColumnValue(column, ColumnType::TEXT, value)};
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
        const std::vector<ColumnName> columns = {ColumnName(column, ColumnType::TEXT)};
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
        const std::vector<ColumnName> columns = {ColumnName("name", ColumnType::TEXT)};
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

bool AgentInfoPersistance::SetGroups(const std::vector<std::string>& groupList)
{
    auto transaction = m_db->BeginTransaction();

    try
    {
        m_db->Remove(AGENT_GROUP_TABLE_NAME);

        for (const auto& group : groupList)
        {
            const Row columns = {ColumnValue("name", ColumnType::TEXT, group)};
            m_db->Insert(AGENT_GROUP_TABLE_NAME, columns);
        }

        m_db->CommitTransaction(transaction);
    }
    catch (const std::exception& e)
    {
        LogError("Error inserting group: {}.", e.what());
        m_db->RollbackTransaction(transaction);
        return false;
    }
    return true;
}

void AgentInfoPersistance::ResetToDefault()
{
    try
    {
        m_db->DropTable(AGENT_INFO_TABLE_NAME);
        m_db->DropTable(AGENT_GROUP_TABLE_NAME);
        CreateAgentInfoTable();
        CreateAgentGroupTable();
        InsertDefaultAgentInfo();
    }
    catch (const std::exception& e)
    {
        LogError("Error resetting to default values: {}.", e.what());
    }
}
