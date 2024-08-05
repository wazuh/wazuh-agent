#pragma once

#include <string>

class AgentInfo
{
public:
    AgentInfo();
    AgentInfo(const std::string& name, const std::string& ip, const std::string& uuid);

    std::string GetName() const;
    std::string GetIP() const;
    std::string GetUUID() const;

    void SetName(const std::string& name);
    void SetIP(const std::string& ip);
    void SetUUID(const std::string& uuid);

private:
    std::string m_name;
    std::string m_ip;
    std::string m_uuid;
};
