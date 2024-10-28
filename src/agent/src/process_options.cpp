#include <process_options.hpp>

#include <agent.hpp>
#include <agent_registration.hpp>

#include <iostream>
#include <string>

void RegisterAgent(const std::string& user,
                   const std::string& password,
                   const std::string& key,
                   const std::string& name,
                   const std::string& configFile)
{
    if (!user.empty() && !password.empty() && !key.empty())
    {
        agent_registration::AgentRegistration reg(user, password, key, name, configFile);

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
        LogError("{}, {}, and {} args are mandatory", OPT_USER, OPT_PASSWORD, OPT_KEY);
    }
}
