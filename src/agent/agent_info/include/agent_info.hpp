#pragma once

#include <string>
#include <vector>

class AgentInfo
{
public:
    AgentInfo();
    AgentInfo(std::string name, std::string key, std::string uuid);

    std::string GetName() const;
    std::string GetKey() const;
    std::string GetUUID() const;
    std::vector<std::string> GetGroups() const;

    void SetName(const std::string& name);
    bool SetKey(const std::string& key);
    void SetUUID(const std::string& uuid);
    void SetGroups(const std::vector<std::string>& groupList);

private:
    std::string CreateKey();
    bool ValidateKey(const std::string& key);

    std::string m_name;
    std::string m_key;
    std::string m_uuid;
    std::vector<std::string> m_groups;
};
