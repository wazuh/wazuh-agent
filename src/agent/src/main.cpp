#include "process_options.hpp"
#include <cmd_ln_parser.hpp>
#include <logger.hpp>

#include <string>

int main(int argc, char* argv[])
{
    Logger logger;

    try
    {
        CommandlineParser cmdParser(argc, argv, validOptions);

        std::string configFile;

        if (cmdParser.OptionExists(OPT_CONFIG_FILE))
        {
            configFile = cmdParser.GetOptionValue(OPT_CONFIG_FILE);
        }

        if (cmdParser.OptionExists(OPT_REGISTER_AGENT))
        {
            RegisterAgent(cmdParser.GetOptionValue(OPT_USER),
                          cmdParser.GetOptionValue(OPT_PASSWORD),
                          cmdParser.GetOptionValue(OPT_KEY),
                          cmdParser.GetOptionValue(OPT_NAME),
                          configFile);
        }
        else if (cmdParser.OptionExists(OPT_RESTART))
        {
            RestartAgent(configFile);
        }
        else if (cmdParser.OptionExists(OPT_STATUS))
        {
            StatusAgent();
        }
        else if (cmdParser.OptionExists(OPT_STOP))
        {
            StopAgent();
        }
        else if (cmdParser.OptionExists(OPT_INSTALL_SERVICE))
        {
            if (!InstallService())
                return 1;
        }
        else if (cmdParser.OptionExists(OPT_REMOVE_SERVICE))
        {
            if (!RemoveService())
                return 1;
        }
        else if (cmdParser.OptionExists(OPT_RUN_SERVICE))
        {
            SetDispatcherThread();
        }
        else if (cmdParser.OptionExists(OPT_HELP))
        {
            PrintHelp();
        }
        else
        {
            StartAgent(configFile);
        }

        return 0;
    }
    catch (const std::exception& e)
    {
        LogCritical("An error occurred: {}.", e.what());
        return 1;
    }
}
