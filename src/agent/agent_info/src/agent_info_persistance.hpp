#pragma once

#include <memory>
#include <string>
#include <vector>

static constexpr char sqlitedb_path[] = "agent_info.db";

namespace SQLite
{
    class Database;
}

class AgentInfoPersistance
{
public:
    explicit AgentInfoPersistance(const std::string& dbPath = sqlitedb_path);
    ~AgentInfoPersistance();

    AgentInfoPersistance(const AgentInfoPersistance&) = delete;
    AgentInfoPersistance& operator=(const AgentInfoPersistance&) = delete;
    AgentInfoPersistance(AgentInfoPersistance&&) = delete;
    AgentInfoPersistance& operator=(AgentInfoPersistance&&) = delete;

    std::string GetName() const;
    std::string GetKey() const;
    std::string GetUUID() const;
    std::vector<std::string> GetGroups() const;

    void SetName(const std::string& name);
    void SetKey(const std::string& key);
    void SetUUID(const std::string& uuid);
    void SetGroups(const std::vector<std::string>& groupList);

    void ResetToDefault();

private:
    bool TableExists(const std::string& table) const;
    bool AgentInfoIsEmpty() const;
    void CreateAgentInfoTable();
    void CreateAgentGroupTable();
    void InsertDefaultAgentInfo();
    void SetAgentInfoValue(const std::string& column, const std::string& value);
    std::string GetAgentInfoValue(const std::string& column) const;

    std::unique_ptr<SQLite::Database> m_db;
};
