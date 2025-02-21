#include <agent_runner.hpp>

#include <config.h>
#include <iostream>
#include <logger.hpp>
#include <process_options.hpp>
#include <restart_handler.hpp>

/// Command-line options
static const auto OPT_HELP {"help"};
static const auto OPT_HELP_DESC {"Display this help menu"};
static const auto OPT_RUN {"run"};
static const auto OPT_RUN_DESC {"Run agent in foreground (this is the default behavior)"};
static const auto OPT_STATUS {"status"};
static const auto OPT_STATUS_DESC {"Check if the agent is running (running or stopped)"};
static const auto OPT_CONFIG_FILE {"config-file"};
static const auto OPT_CONFIG_FILE_DESC {"Path to the Wazuh configuration file (optional)"};
static const auto OPT_REGISTER_AGENT {"register-agent"};
static const auto OPT_REGISTER_AGENT_DESC {"Use this option to register as a new agent"};
static const auto OPT_URL {"url"};
static const auto OPT_URL_DESC {"URL of the server management API"};
static const auto OPT_USER {"user"};
static const auto OPT_USER_DESC {"User to authenticate with the server management API"};
static const auto OPT_PASS {"password"};
static const auto OPT_PASS_DESC {"Password to authenticate with the server management API"};
static const auto OPT_KEY {"key"};
static const auto OPT_KEY_DESC {"Key to register the agent (optional)"};
static const auto OPT_NAME {"name"};
static const auto OPT_NAME_DESC {"Name to register the agent (optional)"};
static const auto OPT_VERIFICATION_MODE {"verification-mode"};
static const auto OPT_VERIFICATION_MODE_DESC {
    "Verification mode to be applied on HTTPS connection to the server (optional)"};

namespace program_options = boost::program_options;

AgentRunner::AgentRunner(int argc, char* argv[])
{
    restart_handler::RestartHandler::SetCommandLineArguments(argc, argv);
    ParseOptions(argc, argv);
}

void AgentRunner::ParseOptions(int argc, char* argv[])
{
    // clang-format off
    cmdParser.add_options()
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

    program_options::store(program_options::parse_command_line(argc, argv, cmdParser), validOptions);
    program_options::notify(validOptions);
}

int AgentRunner::Run() const
{
    if (validOptions.count(OPT_REGISTER_AGENT))
    {
        return RegisterAgent();
    }
    if (validOptions.count(OPT_STATUS))
    {
        StatusAgent(validOptions[OPT_CONFIG_FILE].as<std::string>());
    }
    else if (const auto platformSpecificResult = HandlePlatformSpecificOptions(); platformSpecificResult.has_value())
    {
        return platformSpecificResult.value();
    }
    else if (validOptions.count(OPT_HELP))
    {
        std::cout << cmdParser << '\n';
    }
    else
    {
        StartAgent(validOptions[OPT_CONFIG_FILE].as<std::string>());
    }

    return 0;
}

int AgentRunner::RegisterAgent() const
{
    if (!validOptions.count(OPT_URL) || !validOptions.count(OPT_USER) || !validOptions.count(OPT_PASS))
    {
        std::cout << "--url, --user and --password args are mandatory\n";
        return 1;
    }

    ::RegisterAgent(validOptions[OPT_URL].as<std::string>(),
                    validOptions[OPT_USER].as<std::string>(),
                    validOptions[OPT_PASS].as<std::string>(),
                    validOptions[OPT_KEY].as<std::string>(),
                    validOptions[OPT_NAME].as<std::string>(),
                    validOptions[OPT_CONFIG_FILE].as<std::string>(),
                    validOptions[OPT_VERIFICATION_MODE].as<std::string>());

    return 0;
}
