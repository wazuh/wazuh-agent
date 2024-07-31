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

private:
    bool AgentInfoTableExists() const;
    void CreateAgentInfoTable();

    std::unique_ptr<SQLite::Database> m_db;
};
