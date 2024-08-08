#include <agent.hpp>
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
            const auto name = cmdParser.OptionExists("--name")
                                  ? std::make_optional<std::string>(cmdParser.getOptionValue("--name"))
                                  : std::nullopt;
            const auto ip = cmdParser.OptionExists("--ip")
                                ? std::make_optional<std::string>(cmdParser.getOptionValue("--ip"))
                                : std::nullopt;

            const registration::UserCredentials userCredentials {user, password};
            const registration::AgentInfoOptionalData agentInfoOptionalData {name, ip};

            if (registration::RegisterAgent(userCredentials, agentInfoOptionalData))
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
