#include <agent_runner.hpp>

#include <agent.hpp>
#include <agent_enrollment.hpp>
#include <config.h>
#include <configuration_parser.hpp>
#include <http_client.hpp>
#include <instance_handler.hpp>
#include <logger.hpp>
#include <restart_handler.hpp>

#include <filesystem>
#include <fmt/format.h>

#include <iostream>
#include <string>

namespace program_options = boost::program_options;

namespace
{
    /// Command-line options
    const auto OPT_HELP {"help"};
    const auto OPT_HELP_H {"help,h"};
    const auto OPT_HELP_DESC {"Display this help menu"};
    const auto OPT_RUN {"run"};
    const auto OPT_RUN_DESC {"Run agent in foreground (this is the default behavior)"};
    const auto OPT_STATUS {"status"};
    const auto OPT_STATUS_DESC {"Check if the agent is running (running or stopped)"};
    const auto OPT_CONFIG_FILE {"config-file"};
    const auto OPT_CONFIG_FILE_DESC {"Path to the Wazuh configuration file (optional)"};
    const auto OPT_ENROLL_AGENT {"enroll"};
    const auto OPT_ENROLL_AGENT_DESC {"Use this option to enroll as a new agent"};
    const auto OPT_ENROLL_URL {"enroll-url"};
    const auto OPT_ENROLL_URL_DESC {"URL of the server management API enrollment endpoint"};
    const auto OPT_CONNECT_URL {"connect-url"};
    const auto OPT_CONNECT_URL_DESC {"URL of the server. This option overwrites the server URL in the configuration "
                                     "wazuh-agent.yaml file (optional)"};
    const auto OPT_USER {"user"};
    const auto OPT_USER_DESC {"User to authenticate with the server management API"};
    const auto OPT_PASS {"password"};
    const auto OPT_PASS_DESC {"Password to authenticate with the server management API"};
    const auto OPT_KEY {"key"};
    const auto OPT_KEY_DESC {"Key to enroll the agent (optional)"};
    const auto OPT_NAME {"name"};
    const auto OPT_NAME_DESC {"Name to enroll the agent (optional)"};
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
    m_generalOptions.add_options()
        (OPT_HELP_H, OPT_HELP_DESC)
        (OPT_RUN, OPT_RUN_DESC)
        (OPT_STATUS, OPT_STATUS_DESC)
        (OPT_ENROLL_AGENT, OPT_ENROLL_AGENT_DESC)
        (OPT_CONFIG_FILE, program_options::value<std::string>()->default_value(""), OPT_CONFIG_FILE_DESC);

    m_enrollmentOptions.add_options()
        (OPT_ENROLL_URL, program_options::value<std::string>(), OPT_ENROLL_URL_DESC)
        (OPT_USER, program_options::value<std::string>(), OPT_USER_DESC)
        (OPT_PASS, program_options::value<std::string>(), OPT_PASS_DESC)
        (OPT_CONNECT_URL, program_options::value<std::string>(), OPT_CONNECT_URL_DESC)
        (OPT_KEY, program_options::value<std::string>()->default_value(""), OPT_KEY_DESC)
        (OPT_NAME, program_options::value<std::string>()->default_value(""), OPT_NAME_DESC)
        (OPT_VERIFICATION_MODE, program_options::value<std::string>()->default_value(config::agent::DEFAULT_VERIFICATION_MODE), OPT_VERIFICATION_MODE_DESC);
    // clang-format on

    AddPlatformSpecificOptions();

    m_allOptions.add(m_generalOptions).add(m_enrollmentOptions);

    program_options::store(program_options::parse_command_line(argc, argv, m_allOptions), m_options);
    program_options::notify(m_options);
}

int AgentRunner::Run() const
{
    if (m_options.count(OPT_HELP))
    {
        if (m_options.count(OPT_ENROLL_AGENT))
        {
            std::cout << m_enrollmentOptions << '\n';
        }
        else
        {
            std::cout << m_allOptions << '\n';
        }
    }
    else if (m_options.count(OPT_ENROLL_AGENT))
    {
        return EnrollAgent();
    }
    else if (m_options.count(OPT_STATUS))
    {
        StatusAgent();
    }
    else if (const auto platformSpecificResult = HandlePlatformSpecificOptions(); platformSpecificResult.has_value())
    {
        return platformSpecificResult.value();
    }
    else
    {
        return StartAgent();
    }

    return 0;
}

int AgentRunner::EnrollAgent() const
{
    for (const auto& option : {OPT_ENROLL_URL, OPT_USER, OPT_PASS})
    {
        if (!m_options.count(option) || m_options[option].as<std::string>().empty())
        {
            std::cout << "--enroll-url, --user and --password args are mandatory. Use --help for more information.\n";
            return 1;
        }
    }

    auto configurationParser =
        m_options[OPT_CONFIG_FILE].as<std::string>().empty()
            ? configuration::ConfigurationParser()
            : configuration::ConfigurationParser(std::filesystem::path(m_options[OPT_CONFIG_FILE].as<std::string>()));

    const auto dbFolderPath = configurationParser.GetConfigOrDefault(config::DEFAULT_DATA_PATH, "agent", "path.data");

    if (m_options.count(OPT_CONNECT_URL))
    {
        configurationParser.SetServerURL(m_options[OPT_CONNECT_URL].as<std::string>());
    }

    try
    {
        std::cout << "Starting wazuh-agent enrollment\n";

        agent_enrollment::AgentEnrollment reg(std::make_unique<http_client::HttpClient>(),
                                              m_options[OPT_ENROLL_URL].as<std::string>(),
                                              m_options[OPT_USER].as<std::string>(),
                                              m_options[OPT_PASS].as<std::string>(),
                                              m_options[OPT_KEY].as<std::string>(),
                                              m_options[OPT_NAME].as<std::string>(),
                                              dbFolderPath,
                                              m_options[OPT_VERIFICATION_MODE].as<std::string>());

        if (reg.Enroll())
        {
            std::cout << "wazuh-agent enrolled\n";
        }
        else
        {
            std::cout << "wazuh-agent enrollment failed\n";
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

void AgentRunner::StatusAgent() const
{
    const auto configFilePath = m_options[OPT_CONFIG_FILE].as<std::string>();
    const auto status = instance_handler::GetAgentStatus(configFilePath);
    std::cout << fmt::format("wazuh-agent status: {}\n", status);
}

int AgentRunner::StartAgent() const
{
    try
    {
        Logger::AddPlatformSpecificSink();

        const auto configFilePath = m_options[OPT_CONFIG_FILE].as<std::string>();
        const auto instanceHandler = instance_handler::GetInstanceHandler(configFilePath);

        if (!instanceHandler.isLockAcquired())
        {
            std::cout << "wazuh-agent already running\n";
            return 1;
        }

        LogInfo("Starting wazuh-agent");

        Agent agent(std::make_unique<configuration::ConfigurationParser>(std::filesystem::path(configFilePath)));
        agent.Run();
    }
    catch (const std::exception& e)
    {
        LogError("Exception thrown in wazuh-agent: {}", e.what());
        return 1;
    }

    return 0;
}
