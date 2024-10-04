#include "process_options.hpp"
#include <cmd_ln_parser.hpp>
#include <logger.hpp>

int main(int argc, char* argv[])
{
    Logger logger;
    CommandlineParser cmdParser(argc, argv);

    LogInfo("Starting wazuh-agent service.");

    try
    {
        if (cmdParser.OptionExists("register"))
        {
            RegisterAgent(cmdParser.GetOptionValue("--user"),
                          cmdParser.GetOptionValue("--password"),
                          cmdParser.GetOptionValue("--key"),
                          cmdParser.GetOptionValue("--name"));
        }
        else if (cmdParser.OptionExists("start"))
        {
            StartAgent();
        }
        else if (cmdParser.OptionExists("restart"))
        {
            RestartAgent();
        }
        else if (cmdParser.OptionExists("status"))
        {
            StatusAgent();
        }
        else if (cmdParser.OptionExists("stop"))
        {
            StopAgent();
        }
        else
        {
            PrintHelp();
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        LogCritical("An error occurred: {}.", e.what());
        return 1;
    }
}
