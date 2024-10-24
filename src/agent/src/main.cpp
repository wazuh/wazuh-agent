#include "process_options.hpp"
#include <cmd_ln_parser.hpp>
#include <logger.hpp>

#include <string>

int main(int argc, char* argv[])
{
    Logger logger;
    CommandlineParser cmdParser(argc, argv);

    std::string configPath;

    try
    {
        if (cmdParser.OptionExists("--config-path"))
        {
            configPath = cmdParser.GetOptionValue("--config-path");
        }

        if (cmdParser.OptionExists("--register-agent"))
        {
            RegisterAgent(cmdParser.GetOptionValue("--user"),
                          cmdParser.GetOptionValue("--password"),
                          cmdParser.GetOptionValue("--key"),
                          cmdParser.GetOptionValue("--name"),
                          configPath);
        }
        else if (cmdParser.OptionExists("--restart"))
        {
            RestartAgent(configPath);
        }
        else if (cmdParser.OptionExists("--status"))
        {
            StatusAgent();
        }
        else if (cmdParser.OptionExists("--stop"))
        {
            StopAgent();
        }
        else if (cmdParser.OptionExists("--install-service"))
        {
            if (!InstallService())
                return 1;
        }
        else if (cmdParser.OptionExists("--remove-service"))
        {
            if (!RemoveService())
                return 1;
        }
        else if (cmdParser.OptionExists("--run-service"))
        {
            SetDispatcherThread();
        }
        else if (cmdParser.OptionExists("--run") || cmdParser.OptionExists("--start"))
        {
            StartAgent(configPath);
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
