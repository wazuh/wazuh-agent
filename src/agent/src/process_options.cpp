#include <process_options.hpp>

#include <agent.hpp>
#include <agent_registration.hpp>
#include <config.h>
#include <http_client.hpp>

#include <iostream>
#include <string>

void RegisterAgent(const std::string& url,
                   const std::string& user,
                   const std::string& password,
                   const std::string& key,
                   const std::string& name,
                   const std::string& configFile)
{
    auto configurationParser = configFile.empty()
                                   ? configuration::ConfigurationParser()
                                   : configuration::ConfigurationParser(std::filesystem::path(configFile));
    auto dbFolderPath =
        configurationParser.GetConfig<std::string>("agent", "path.data").value_or(config::DEFAULT_DATA_PATH);

    if (!url.empty() && !user.empty() && !password.empty())
    {
        try
        {
            std::cout << "Starting wazuh-agent registration\n";

            agent_registration::AgentRegistration reg(url, user, password, key, name, dbFolderPath);

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
        std::cout << OPT_URL << ", " << OPT_USER << " and " << OPT_PASSWORD << " args are mandatory\n";
    }
}
