#pragma once

#include <string>

class AgentInfo
{
public:
    AgentInfo();
    AgentInfo(const std::string& name, const std::string& key, const std::string& uuid);

    std::string GetName() const;
    std::string GetKey() const;
    std::string GetUUID() const;

    void SetName(const std::string& name);
    void SetKey(const std::string& key);
    void SetUUID(const std::string& uuid);

private:
    std::string m_name;
    std::string m_key;
    std::string m_uuid;
};
