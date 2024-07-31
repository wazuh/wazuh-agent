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

void AgentInfoPersistance::CreateAgentInfoTable()
{
    try
    {
        m_db->exec("CREATE TABLE IF NOT EXISTS agent_info ("
                   "id INTEGER PRIMARY KEY, "
                   "name TEXT, "
                   "ip TEXT, "
                   "uuid TEXT"
                   ");");
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error creating table: " << e.what() << std::endl;
    }
}

