#include <agent_info.hpp>

#include <agent_info_persistance.hpp>

#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

AgentInfo::AgentInfo()
{
    AgentInfoPersistance agentInfoPersistance;
    m_name = agentInfoPersistance.GetName();
    m_ip = agentInfoPersistance.GetIP();
    m_uuid = agentInfoPersistance.GetUUID();

    if (m_uuid.empty())
    {
        AgentInfoPersistance agentInfoPersistance;
        m_uuid = boost::uuids::to_string(boost::uuids::random_generator()());
        agentInfoPersistance.SetUUID(m_uuid);
    }
}

AgentInfo::AgentInfo(const std::string& name, const std::string& ip, const std::string& uuid)
    : m_name(name)
    , m_ip(ip)
    , m_uuid(uuid)
{
    AgentInfoPersistance agentInfoPersistance;
    agentInfoPersistance.SetName(m_name);
    agentInfoPersistance.SetIP(m_ip);
    agentInfoPersistance.SetUUID(m_uuid);
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
