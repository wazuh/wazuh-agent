#include <process_options.hpp>

#include <agent.hpp>
#include <agent_registration.hpp>

#include <iostream>
#include <string>

void RegisterAgent(const std::string& user,
                   const std::string& password,
                   const std::string& key,
                   const std::string& name,
                   const std::string& configPath)
{
    if (!user.empty() && !password.empty() && !key.empty())
    {
        agent_registration::AgentRegistration reg(user, password, key, name, configPath);

        http_client::HttpClient httpClient;
        if (reg.Register(httpClient))
        {
            LogInfo("wazuh-agent registered.");
        }
        else
        {
            LogError("wazuh-agent registration fail.");
        }
    }
    else
    {
        LogError("--user, --password and --key args are mandatory");
    }
}
