#include <agent_info.hpp>

#include <agent_info_persistance.hpp>

#include <boost/uuid/uuid.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <random>
#include <utility>

namespace
{
    constexpr size_t KEY_LENGTH = 32;
    const std::string AGENT_TYPE = "Endpoint";
    const std::string AGENT_VERSION = "5.0.0";
} // namespace

AgentInfo::AgentInfo()
{
    AgentInfoPersistance agentInfoPersistance;
    m_key = agentInfoPersistance.GetKey();
    m_uuid = agentInfoPersistance.GetUUID();
    m_groups = agentInfoPersistance.GetGroups();

    if (m_uuid.empty())
    {
        m_uuid = boost::uuids::to_string(boost::uuids::random_generator()());
        agentInfoPersistance.SetUUID(m_uuid);
    }
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

bool AgentInfo::SetKey(const std::string& key)
{
    AgentInfoPersistance agentInfoPersistance;

    if (!key.empty())
    {
        if (!ValidateKey(key))
        {
            return false;
        }
        m_key = key;
    }
    else
    {
        m_key = CreateKey();
    }

    agentInfoPersistance.SetKey(m_key);

    return true;
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

std::string AgentInfo::CreateKey() const
{
    constexpr std::string_view charset = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";

    std::random_device rd;
    std::mt19937 generator(rd());
    std::uniform_int_distribution<size_t> distribution(0, charset.size() - 1);

    std::string key;
    for (size_t i = 0; i < KEY_LENGTH; ++i)
    {
        key += charset[distribution(generator)];
    }

    return key;
}

bool AgentInfo::ValidateKey(const std::string& key) const
{
    return key.length() == KEY_LENGTH && std::ranges::all_of(key, ::isalnum);
}

std::string AgentInfo::GetType() const
{
    return AGENT_TYPE;
}

std::string AgentInfo::GetVersion() const
{
    return AGENT_VERSION;
}

nlohmann::json AgentInfo::GetEndpointInfo() const
{
    return nlohmann::json::parse(
        R"({"os": "Amazon Linux 2","platform": "Linux","ip": "192.168.1.2","arch": "x86_64"})");
}

std::string AgentInfo::GetHeaderInfo() const
{
    return "WazuhXDR/5.0.0 (Endpoint; x86_64; Linux)";
}

nlohmann::json AgentInfo::GetMetadataInfo([[maybe_unused]] const bool includeKey) const
{
    return nlohmann::json::parse(
        R"({"os": "Amazon Linux 2",
        "platform": "Linux",
        "ip": "192.168.1.2",
        "arch": "x86_64",
        "type": "Endpoint",
        "version": "5.0.0",
        "groups": ["pepa", "pepe"],
        "uuid": "skjrowegj12355",
        "key": "key"})");
}
