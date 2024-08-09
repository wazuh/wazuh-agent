#include <agent.hpp>
#include <agent_info.hpp>
#include <cmd_ln_parser.hpp>
#include <register.hpp>

#include <iostream>
#include <optional>

int main(int argc, char* argv[])
{
    CommandlineParser cmdParser(argc, argv);

    if (cmdParser.OptionExists("--register"))
    {
        std::cout << "Starting registration process" << std::endl;

        if (cmdParser.OptionExists("--user") && cmdParser.OptionExists("--password"))
        {
            const auto user = cmdParser.getOptionValue("--user");
            const auto password = cmdParser.getOptionValue("--password");

            AgentInfo agentInfo;

            if (cmdParser.OptionExists("--name"))
            {
                agentInfo.SetName(cmdParser.getOptionValue("--name"));
            }

            if (cmdParser.OptionExists("--ip"))
            {
                agentInfo.SetIP(cmdParser.getOptionValue("--ip"));
            }

            const registration::UserCredentials userCredentials {user, password};

            if (registration::RegisterAgent(userCredentials))
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
            std::cout << "--user and --password args are mandatory" << std::endl;
        }

        std::cout << "Exiting ..." << std::endl;
        return 0;
    }

    Agent agent;
    agent.Run();
}
