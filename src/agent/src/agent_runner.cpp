#include <agent_runner.hpp>

#include <agent_registration.hpp>
#include <config.h>
#include <configuration_parser.hpp>
#include <http_client.hpp>
#include <logger.hpp>
#include <process_options.hpp>
#include <restart_handler.hpp>

#include <iostream>
#include <string>

namespace program_options = boost::program_options;

namespace
{
    /// Command-line options
    const auto OPT_HELP {"help"};
    const auto OPT_HELP_DESC {"Display this help menu"};
    const auto OPT_RUN {"run"};
    const auto OPT_RUN_DESC {"Run agent in foreground (this is the default behavior)"};
    const auto OPT_STATUS {"status"};
    const auto OPT_STATUS_DESC {"Check if the agent is running (running or stopped)"};
    const auto OPT_CONFIG_FILE {"config-file"};
    const auto OPT_CONFIG_FILE_DESC {"Path to the Wazuh configuration file (optional)"};
    const auto OPT_REGISTER_AGENT {"register-agent"};
    const auto OPT_REGISTER_AGENT_DESC {"Use this option to register as a new agent"};
    const auto OPT_URL {"url"};
    const auto OPT_URL_DESC {"URL of the server management API"};
    const auto OPT_USER {"user"};
    const auto OPT_USER_DESC {"User to authenticate with the server management API"};
    const auto OPT_PASS {"password"};
    const auto OPT_PASS_DESC {"Password to authenticate with the server management API"};
    const auto OPT_KEY {"key"};
    const auto OPT_KEY_DESC {"Key to register the agent (optional)"};
    const auto OPT_NAME {"name"};
    const auto OPT_NAME_DESC {"Name to register the agent (optional)"};
    const auto OPT_VERIFICATION_MODE {"verification-mode"};
    const auto OPT_VERIFICATION_MODE_DESC {
        "Verification mode to be applied on HTTPS connection to the server (optional)"};
} // namespace

AgentRunner::AgentRunner(int argc, char* argv[])
{
    restart_handler::RestartHandler::SetCommandLineArguments(argc, argv);
    ParseOptions(argc, argv);
}

void AgentRunner::ParseOptions(int argc, char* argv[])
{
    // clang-format off
    m_desc.add_options()
        (OPT_HELP, OPT_HELP_DESC)
        (OPT_RUN, OPT_RUN_DESC)
        (OPT_STATUS, OPT_STATUS_DESC)
        (OPT_CONFIG_FILE, program_options::value<std::string>()->default_value(""), OPT_CONFIG_FILE_DESC)
        (OPT_REGISTER_AGENT, OPT_REGISTER_AGENT_DESC)
        (OPT_URL, program_options::value<std::string>(), OPT_URL_DESC)
        (OPT_USER, program_options::value<std::string>(), OPT_USER_DESC)
        (OPT_PASS, program_options::value<std::string>(), OPT_PASS_DESC)
        (OPT_KEY, program_options::value<std::string>()->default_value(""), OPT_KEY_DESC)
        (OPT_NAME, program_options::value<std::string>()->default_value(""), OPT_NAME_DESC)
        (OPT_VERIFICATION_MODE, program_options::value<std::string>()->default_value(config::agent::DEFAULT_VERIFICATION_MODE), OPT_VERIFICATION_MODE_DESC);
    // clang-format on

    AddPlatformSpecificOptions();

    program_options::store(program_options::parse_command_line(argc, argv, m_desc), m_options);
    program_options::notify(m_options);
}

int AgentRunner::Run() const
{
    if (m_options.count(OPT_REGISTER_AGENT))
    {
        return RegisterAgent();
    }
    if (m_options.count(OPT_STATUS))
    {
        StatusAgent(m_options[OPT_CONFIG_FILE].as<std::string>());
    }
    else if (const auto platformSpecificResult = HandlePlatformSpecificOptions(); platformSpecificResult.has_value())
    {
        return platformSpecificResult.value();
    }
    else if (m_options.count(OPT_HELP))
    {
        std::cout << m_desc << '\n';
    }
    else
    {
        StartAgent(m_options[OPT_CONFIG_FILE].as<std::string>());
    }

    return 0;
}

int AgentRunner::RegisterAgent() const
{
    for (const auto& option : {OPT_URL, OPT_USER, OPT_PASS})
    {
        if (!m_options.count(option) || m_options[option].as<std::string>().empty())
        {
            std::cout << "--url, --user and --password args are mandatory\n";
            return 1;
        }
    }

    const auto configurationParser =
        m_options[OPT_CONFIG_FILE].as<std::string>().empty()
            ? configuration::ConfigurationParser()
            : configuration::ConfigurationParser(std::filesystem::path(m_options[OPT_CONFIG_FILE].as<std::string>()));

    const auto dbFolderPath = configurationParser.GetConfigOrDefault(config::DEFAULT_DATA_PATH, "agent", "path.data");

    try
    {
        std::cout << "Starting wazuh-agent registration\n";

        agent_registration::AgentRegistration reg(std::make_unique<http_client::HttpClient>(),
                                                  m_options[OPT_URL].as<std::string>(),
                                                  m_options[OPT_USER].as<std::string>(),
                                                  m_options[OPT_PASS].as<std::string>(),
                                                  m_options[OPT_KEY].as<std::string>(),
                                                  m_options[OPT_NAME].as<std::string>(),
                                                  dbFolderPath,
                                                  m_options[OPT_VERIFICATION_MODE].as<std::string>());

        if (reg.Register())
        {
            std::cout << "wazuh-agent registered\n";
        }
        else
        {
            std::cout << "wazuh-agent registration failed\n";
            return 1;
        }
    }
    catch (const std::exception& e)
    {
        std::cerr << "Error: " << e.what() << '\n';
        return 1;
    }

    return 0;
}
