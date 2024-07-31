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

void AgentInfo::SetName(const std::string& name)
{
    AgentInfoPersistance agentInfoPersistance;
    agentInfoPersistance.SetName(name);
    m_name = name;
}

void AgentInfo::SetIP(const std::string& ip)
{
    AgentInfoPersistance agentInfoPersistance;
    agentInfoPersistance.SetIP(ip);
    m_ip = ip;
}

void AgentInfo::SetUUID(const std::string& uuid)
{
    AgentInfoPersistance agentInfoPersistance;
    agentInfoPersistance.SetUUID(uuid);
    m_uuid = uuid;
}
