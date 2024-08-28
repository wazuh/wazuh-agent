#include <agent.hpp>
#include <agent_info.hpp>
#include <cmd_ln_parser.hpp>
#include <http_client.hpp>
#include <register.hpp>

#include <iostream>
#include <optional>

int main(int argc, char* argv[])
{
    CommandlineParser cmdParser(argc, argv);

    if (cmdParser.OptionExists("--register"))
    {
        std::cout << "Starting registration process" << std::endl;

        if (cmdParser.OptionExists("--user") && cmdParser.OptionExists("--password") && cmdParser.OptionExists("--key"))
        {
            const auto user = cmdParser.GetOptionValue("--user");
            const auto password = cmdParser.GetOptionValue("--password");

            AgentInfo agentInfo;
            agentInfo.SetKey(cmdParser.GetOptionValue("--key"));

            if (cmdParser.OptionExists("--name"))
            {
                agentInfo.SetName(cmdParser.GetOptionValue("--name"));
            }

            http_client::HttpClient httpClient;
            const registration::UserCredentials userCredentials {user, password};

            if (registration::RegisterAgent(userCredentials, httpClient))
            {
                std::cout << "Agent registered." << std::endl;
            }
            else
            {
                std::cout << "Registration fail." << std::endl;
            }
        }
        else
        {
            std::cout << "--user, --password and --key args are mandatory" << std::endl;
        }

        std::cout << "Exiting ..." << std::endl;
        return 0;
    }

    Agent agent;
    agent.Run();
}
