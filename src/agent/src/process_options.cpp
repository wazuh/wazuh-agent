#include <process_options.hpp>

#include <agent.hpp>
#include <agent_registration.hpp>
#include <config.h>
#include <fmt/format.h>
#include <http_client.hpp>
#include <instance_handler.hpp>

#include <iostream>
#include <string>

void RegisterAgent(const std::string& url,
                   const std::string& user,
                   const std::string& password,
                   const std::string& key,
                   const std::string& name,
                   const std::string& configFilePath,
                   const std::string& verificationMode)
{
    auto configurationParser = configFilePath.empty()
                                   ? configuration::ConfigurationParser()
                                   : configuration::ConfigurationParser(std::filesystem::path(configFilePath));
    auto dbFolderPath =
        configurationParser.GetConfig<std::string>("agent", "path.data").value_or(config::DEFAULT_DATA_PATH);

    if (!url.empty() && !user.empty() && !password.empty())
    {
        try
        {
            std::cout << "Starting wazuh-agent registration\n";

            agent_registration::AgentRegistration reg(std::make_unique<http_client::HttpClient>(),
                                                      url,
                                                      user,
                                                      password,
                                                      key,
                                                      name,
                                                      dbFolderPath,
                                                      verificationMode);

            if (reg.Register())
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
        std::cout << "--url, --user and --password args are mandatory\n";
    }
}

void StatusAgent(const std::string& configFilePath)
{
    std::cout << fmt::format("wazuh-agent status: {}\n", instance_handler::GetAgentStatus(configFilePath));
}
