#pragma once

#include <string>

class AgentInfo
{
public:
    AgentInfo();

private:
    std::string m_name;
    std::string m_ip;
    std::string m_uuid;
};
