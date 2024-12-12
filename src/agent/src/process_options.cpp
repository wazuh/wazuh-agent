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
                   const std::string& configFilePath)
{
    auto configurationParser = configFilePath.empty()
                                   ? configuration::ConfigurationParser()
                                   : configuration::ConfigurationParser(std::filesystem::path(configFilePath));
    auto dbFolderPath =
        configurationParser.GetConfig<std::string>("agent", "path.data").value_or(config::DEFAULT_DATA_PATH);

    auto verificationMode = configurationParser.GetConfig<std::string>("agent", "verification_mode")
                                .value_or(config::agent::DEFAULT_VERIFICATION_MODE);

    if (!url.empty() && !user.empty() && !password.empty())
    {
        try
        {
            std::cout << "Starting wazuh-agent registration\n";

            if (std::find(std::begin(config::agent::VALID_VERIFICATION_MODES),
                          std::end(config::agent::VALID_VERIFICATION_MODES),
                          verificationMode) == std::end(config::agent::VALID_VERIFICATION_MODES))
            {
                LogWarn("Incorrect value for 'verification_mode', in case of HTTPS connections the default value '{}' "
                        "is used.",
                        config::agent::DEFAULT_VERIFICATION_MODE);
                verificationMode = config::agent::DEFAULT_VERIFICATION_MODE;
            }

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
        std::cout << "--url, --user and --password args are mandatory\n";
    }
}
