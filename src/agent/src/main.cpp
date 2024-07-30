#include "agent.hpp"
#include "cmd_ln_parser.hpp"
#include "register.hpp"

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
            std::optional<std::string> name;
            std::optional<std::string> ip;

            if (cmdParser.OptionExists("--name"))
            {
                name = cmdParser.getOptionValue("--name");
            }

            if (cmdParser.OptionExists("--ip"))
            {
                ip = cmdParser.getOptionValue("--ip");
            }

            if (RegisterAgent(user, password, name, ip))
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
    std::this_thread::sleep_for(std::chrono::seconds(15));
}
