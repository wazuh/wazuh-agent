#pragma once

#include <memory>
#include <string>

static constexpr char sqlitedb_path[] = "agent_info.db";

namespace SQLite
{
    class Database;
}

class AgentInfoPersistance
{
public:
    explicit AgentInfoPersistance(const std::string& db_path = sqlitedb_path);
    ~AgentInfoPersistance();

    AgentInfoPersistance(const AgentInfoPersistance&) = delete;
    AgentInfoPersistance& operator=(const AgentInfoPersistance&) = delete;
    AgentInfoPersistance(AgentInfoPersistance&&) = delete;
    AgentInfoPersistance& operator=(AgentInfoPersistance&&) = delete;

    std::string GetName() const;
    std::string GetIP() const;
    std::string GetUUID() const;

    void ResetToDefault();

private:
    bool AgentInfoTableExists() const;
    void CreateAgentInfoTable();
    void InsertDefaultAgentInfo();
    std::string GetAgentInfoValue(const std::string& column) const;

    std::unique_ptr<SQLite::Database> m_db;
};
