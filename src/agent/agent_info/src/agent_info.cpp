#include <agent_info.hpp>

#include <agent_info_persistance.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

AgentInfo::AgentInfo()
{
    AgentInfoPersistance agentInfoPersistance;
    m_name = agentInfoPersistance.GetName();
    m_key = agentInfoPersistance.GetKey();
    m_uuid = agentInfoPersistance.GetUUID();

    if (m_uuid.empty())
    {
        AgentInfoPersistance agentInfoPersistance;
        m_uuid = boost::uuids::to_string(boost::uuids::random_generator()());
        agentInfoPersistance.SetUUID(m_uuid);
    }
}

AgentInfo::AgentInfo(const std::string& name, const std::string& key, const std::string& uuid)
    : m_name(name)
    , m_key(key)
    , m_uuid(uuid)
{
    AgentInfoPersistance agentInfoPersistance;
    agentInfoPersistance.SetName(m_name);
    agentInfoPersistance.SetKey(m_key);
    agentInfoPersistance.SetUUID(m_uuid);
}

std::string AgentInfo::GetName() const
{
    return m_name;
}
std::string AgentInfo::GetKey() const
{
    return m_key;
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

void AgentInfo::SetKey(const std::string& key)
{
    AgentInfoPersistance agentInfoPersistance;
    agentInfoPersistance.SetKey(key);
    m_key = key;
}

void AgentInfo::SetUUID(const std::string& uuid)
{
    AgentInfoPersistance agentInfoPersistance;
    agentInfoPersistance.SetUUID(uuid);
    m_uuid = uuid;
}
