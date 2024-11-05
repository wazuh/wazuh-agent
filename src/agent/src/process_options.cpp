#include <process_options.hpp>

#include <agent.hpp>
#include <agent_registration.hpp>
#include <http_client.hpp>

#include <iostream>
#include <string>

void RegisterAgent(const std::string& user,
                   const std::string& password,
                   const std::string& key,
                   const std::string& name,
                   const std::string& configFile)
{
    if (!user.empty() && !password.empty())
    {
        try
        {
            std::cout << "Starting wazuh-agent registration\n";

            agent_registration::AgentRegistration reg(user, password, key, name, configFile);

            http_client::HttpClient httpClient;
            if (reg.Register(httpClient))
            {
                std::cout << "wazuh-agent registered\n";
            }
            else
            {
                std::cout << "wazuh-agent registration failed\n";
            }
        }
        catch (const std::exception& e)
        {
            std::cerr << "Error: " << e.what() << '\n';
        }
    }
    else
    {
        std::cout << OPT_USER << " and " << OPT_PASSWORD << " args are mandatory\n";
    }
}
