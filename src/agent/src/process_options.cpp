#include <process_options.hpp>

#include <agent.hpp>
#include <agent_registration.hpp>

#include <iostream>
#include <string>

void RegisterAgent(const std::string& user,
                   const std::string& password,
                   const std::string& key,
                   const std::string& name)
{
    if (!user.empty() && !password.empty() && !key.empty())
    {
        agent_registration::AgentRegistration reg(user, password, key, name);

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

void PrintHelp()
{
    LogInfo("Wazuh Agent help.");

    // TO DO
    std::cout << "wazuh-agent [start/status/stop]\n";
    std::cout << "     start           Start wazuh-agent daemon\n";
    std::cout << "     status          Get wazuh-agent daemon status\n";
    std::cout << "     stop            Stop wazuh-agent daemon\n";
    std::cout << "     restart         Restart wazuh-agent daemon\n";
    std::cout << "     register        Register wazuh-agent daemon\n";
    std::cout << "     --help          This help message\n";
}
