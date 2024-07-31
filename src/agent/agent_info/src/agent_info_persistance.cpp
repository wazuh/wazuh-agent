#include <agent_info_persistance.hpp>

#include <SQLiteCpp/SQLiteCpp.h>

#include <iostream>

AgentInfoPersistance::AgentInfoPersistance(const std::string& db_path)
{
    try
    {
        m_db = std::make_unique<SQLite::Database>(db_path, SQLite::OPEN_READWRITE | SQLite::OPEN_CREATE);
        CreateAgentInfoTable();
    }
    catch (const std::exception& e)
    {
        std::cerr << "Can't open database: " << e.what() << std::endl;
        m_db.reset();
    }
}

AgentInfoPersistance::~AgentInfoPersistance() = default;

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

