#include <agent_info.hpp>

#include <agent_info_persistance.hpp>

AgentInfo::AgentInfo()
{
    AgentInfoPersistance agentInfoPersistance;
    m_name = agentInfoPersistance.GetName();
    m_ip = agentInfoPersistance.GetIP();
    m_uuid = agentInfoPersistance.GetUUID();
}
