#include <agent.hpp>
#include <agent_info.hpp>
#include <agent_registration.hpp>
#include <cmd_ln_parser.hpp>
#include <http_client.hpp>
#include <logger.hpp>

#include <optional>

int main(int argc, char* argv[])
{
    Logger logger;
    LogInfo("Starting Wazuh Agent.");

    try
    {
        CommandlineParser cmdParser(argc, argv);

        if (cmdParser.OptionExists("--register"))
        {
            LogInfo("Starting registration process");

            if (cmdParser.OptionExists("--user") && cmdParser.OptionExists("--password") &&
                cmdParser.OptionExists("--key"))
            {
                const auto user = cmdParser.GetOptionValue("--user");
                const auto password = cmdParser.GetOptionValue("--password");

                AgentInfo agentInfo;
                agentInfo.SetKey(cmdParser.GetOptionValue("--key"));

                if (cmdParser.OptionExists("--name"))
                {
                    agentInfo.SetName(cmdParser.GetOptionValue("--name"));
                }
                else
                {
                    agentInfo.SetName(boost::asio::ip::host_name());
                }

                http_client::HttpClient httpClient;

                agent_registration::AgentRegistration reg(user, password, agentInfo.GetKey(), agentInfo.GetName());
                if (reg.Register(httpClient))
                {
                    LogInfo("Agent registered.");
                }
                else
                {
                    LogError("Registration fail.");
                }
            }
            else
            {
                LogError("--user, --password and --key args are mandatory");
            }

            LogInfo("Exiting ...");
            return 0;
        }
    }
    catch (const std::exception& e)
    {
        LogError("An error occurred: {}.", e.what());
        return 1;
    }

    Agent agent;
    agent.Run();
}
