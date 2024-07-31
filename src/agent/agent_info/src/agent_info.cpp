#include <agent_info.hpp>

#include <agent_info_persistance.hpp>

AgentInfo::AgentInfo()
{
    AgentInfoPersistance agentInfoPersistance;
    m_name = agentInfoPersistance.GetName();
    m_ip = agentInfoPersistance.GetIP();
    m_uuid = agentInfoPersistance.GetUUID();
}

std::string AgentInfo::GetName() const
{
    return m_name;
}
std::string AgentInfo::GetIP() const
{
    return m_ip;
}
std::string AgentInfo::GetUUID() const
{
    return m_uuid;
}
