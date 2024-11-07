#include <agent_info.hpp>

#include <agent_info_persistance.hpp>
#include <sysInfo.hpp>

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
    const std::string PRODUCT_NAME = "WazuhXDR";
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

    LoadEndpointInfo();
    LoadMetadataInfo();
    LoadHeaderInfo();
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
    return m_endpointInfo;
}

std::string AgentInfo::GetHeaderInfo() const
{
    return m_headerInfo;
}

nlohmann::json AgentInfo::GetMetadataInfo(const bool includeKey) const
{
    nlohmann::json metadataInfo = m_metadataInfo;

    if (includeKey)
    {
        metadataInfo["key"] = GetKey();
    }

    return metadataInfo;
}

std::optional<std::string> AgentInfo::GetActiveIPAddress(const nlohmann::json& networksJson) const
{
    if (networksJson.contains("iface"))
    {
        for (const auto& iface : networksJson["iface"])
        {
            if (iface.contains("state") && iface["state"] == "up")
            {
                if (iface.contains("IPv4") && !iface["IPv4"].empty())
                {
                    return iface["IPv4"][0].value("address", "");
                }
            }
        }
    }
    return std::nullopt;
}

void AgentInfo::LoadEndpointInfo()
{
    std::shared_ptr<SysInfo> spInfo = std::make_shared<SysInfo>();

    nlohmann::json osInfo = spInfo->os();
    nlohmann::json networksInfo = spInfo->networks();

    m_endpointInfo["os"] = osInfo.value("os_name", "Unknown");
    m_endpointInfo["platform"] = osInfo.value("sysname", "Unknown");
    m_endpointInfo["arch"] = osInfo.value("architecture", "Unknown");

    auto ipAddress = GetActiveIPAddress(networksInfo);
    m_endpointInfo["ip"] = ipAddress.value_or("Unknown");
}

void AgentInfo::LoadMetadataInfo()
{
    m_metadataInfo["os"] = m_endpointInfo.value("os", "Unknown");
    m_metadataInfo["platform"] = m_endpointInfo.value("platform", "Unknown");
    m_metadataInfo["ip"] = m_endpointInfo.value("ip", "Unknown");
    m_metadataInfo["arch"] = m_endpointInfo.value("arch", "Unknown");

    m_metadataInfo["type"] = GetType();
    m_metadataInfo["version"] = GetVersion();
    m_metadataInfo["groups"] = GetGroups();
    m_metadataInfo["uuid"] = GetUUID();
}

void AgentInfo::LoadHeaderInfo()
{
    m_headerInfo = PRODUCT_NAME + "/" + GetVersion() + " (" + GetType() + "; " +
                   m_endpointInfo.value("arch", "Unknown") + "; " + m_endpointInfo.value("platform", "Unknown") + ")";
}
