#pragma once

#include <agent_info.hpp>
#include <configuration_parser.hpp>
#include <http_client.hpp>

#include <string>

namespace agent_registration
{
    struct UserCredentials
    {
        std::string user;
        std::string password;
    };

    class AgentRegistration
    {
    public:
        AgentRegistration(std::string user,
                          std::string password,
                          const std::string& key,
                          const std::string& name,
                          std::optional<std::string> configFile);
        bool Register(http_client::IHttpClient& httpClient);

    private:
        configuration::ConfigurationParser m_configurationParser;
        std::string m_serverUrl;
        std::string m_registrationUrl;
        std::string m_user;
        std::string m_password;
        AgentInfo m_agentInfo;
    };
} // namespace agent_registration
