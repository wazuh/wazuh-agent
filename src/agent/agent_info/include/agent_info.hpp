#pragma once

#include <string>

class AgentInfo
{
public:
    AgentInfo();

    std::string GetName() const;
    std::string GetIP() const;
    std::string GetUUID() const;

private:
    std::string m_name;
    std::string m_ip;
    std::string m_uuid;
};
