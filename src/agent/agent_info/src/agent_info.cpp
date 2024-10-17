#include <agent_info.hpp>

#include <agent_info_persistance.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <utility>

AgentInfo::AgentInfo()
{
    AgentInfoPersistance agentInfoPersistance;
    m_name = agentInfoPersistance.GetName();
    m_key = agentInfoPersistance.GetKey();
    m_uuid = agentInfoPersistance.GetUUID();
    m_groups = agentInfoPersistance.GetGroups();

    if (m_uuid.empty())
    {
        m_uuid = boost::uuids::to_string(boost::uuids::random_generator()());
        agentInfoPersistance.SetUUID(m_uuid);
    }
}

AgentInfo::AgentInfo(std::string name, std::string key, std::string uuid)
    : m_name(std::move(name))
    , m_key(std::move(key))
    , m_uuid(std::move(uuid))
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

std::vector<std::string> AgentInfo::GetGroups() const
{
    return m_groups;
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

void AgentInfo::SetGroups(const std::vector<std::string>& groupList)
{
    AgentInfoPersistance agentInfoPersistance;
    agentInfoPersistance.SetGroups(groupList);
    m_groups = groupList;
}
