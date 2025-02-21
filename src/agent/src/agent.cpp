#include <agent.hpp>

#include <command_entry.hpp>
#include <command_handler_utils.hpp>
#include <config.h>
#include <http_client.hpp>
#include <message.hpp>
#include <message_queue_utils.hpp>
#include <multitype_queue.hpp>
#include <restart_handler.hpp>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <memory>

Agent::Agent(const std::string& configFilePath,
             std::unique_ptr<ISignalHandler> signalHandler,
             std::unique_ptr<http_client::IHttpClient> httpClient,
             std::optional<AgentInfo> agentInfo)
    : m_signalHandler(std::move(signalHandler))
    , m_configurationParser(configFilePath.empty() ? std::make_shared<configuration::ConfigurationParser>()
                                                   : std::make_shared<configuration::ConfigurationParser>(
                                                         std::filesystem::path(configFilePath)))
    , m_agentInfo(agentInfo.has_value()
                      ? std::move(*agentInfo)
                      : AgentInfo(
                            m_configurationParser->GetConfigOrDefault(config::DEFAULT_DATA_PATH, "agent", "path.data"),
                            [this]() { return m_sysInfo.os(); },
                            [this]() { return m_sysInfo.networks(); }))
    , m_messageQueue(std::make_shared<MultiTypeQueue>(m_configurationParser))
    , m_communicator(httpClient ? std::move(httpClient) : std::make_unique<http_client::HttpClient>(),
                     m_configurationParser,
                     m_agentInfo.GetUUID(),
                     m_agentInfo.GetKey(),
                     [this]() { return m_agentInfo.GetHeaderInfo(); })
    , m_moduleManager([this](Message message) -> int { return m_messageQueue->push(std::move(message)); },
                      m_configurationParser,
                      m_agentInfo.GetUUID())
    , m_commandHandler(m_configurationParser)
    , m_centralizedConfiguration(
          [this](const std::vector<std::string>& groups)
          {
              m_agentInfo.SetGroups(groups);
              return m_agentInfo.SaveGroups();
          },
          [this]() { return m_agentInfo.GetGroups(); },
          // NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)
          [this](std::string groupId, std::string destinationPath) -> boost::asio::awaitable<bool> {
              co_return co_await m_communicator.GetGroupConfigurationFromManager(std::move(groupId),
                                                                                 std::move(destinationPath));
          },
          // NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)
          [this](const std::filesystem::path& fileToValidate)
          { return m_configurationParser->isValidYamlFile(fileToValidate); },
          [this]() { ReloadModules(); })
{
    // Check if agent is enrolled
    if (m_agentInfo.GetName().empty() || m_agentInfo.GetKey().empty() || m_agentInfo.GetUUID().empty())
    {
        throw std::runtime_error("The agent is not enrolled");
    }

    m_configurationParser->SetGetGroupIdsFunction([this]() { return m_agentInfo.GetGroups(); });

    m_agentThreadCount =
        m_configurationParser->GetConfigInRangeOrDefault<size_t>(config::DEFAULT_THREAD_COUNT,
                                                                 std::optional<size_t>(config::DEFAULT_THREAD_COUNT),
                                                                 std::optional<size_t> {},
                                                                 "agent",
                                                                 "thread_count");
}

Agent::~Agent()
{
    m_taskManager.Stop();
}

void Agent::ReloadModules()
{
    const std::lock_guard<std::mutex> lock(m_reloadMutex);

    if (m_running.load())
    {
        try
        {
            LogInfo("Reloading Modules");
            m_configurationParser->ReloadConfiguration();
            m_moduleManager.Stop();
            m_moduleManager.Setup();
            m_moduleManager.Start();
            LogInfo("Modules reloaded");
        }
        catch (const std::exception& e)
        {
            LogError("Error reloading modules: {}", e.what());
        }
    }
    else
    {
        LogWarn("Agent cannot reload modules while start up or shutdown is in progress.");
    }
}

void Agent::Run()
{
    m_taskManager.Start(m_agentThreadCount);

    // Check if the server recognizes the agent
    m_communicator.SendAuthenticationRequest();

    m_taskManager.EnqueueTask(m_communicator.WaitForTokenExpirationAndAuthenticate(), "Authenticate");

    m_taskManager.EnqueueTask(m_communicator.GetCommandsFromManager([this](const int, const std::string& response)
                                                                    { PushCommandsToQueue(m_messageQueue, response); }),
                              "FetchCommands");

    m_taskManager.EnqueueTask(m_communicator.StatefulMessageProcessingTask(
                                  [this](const size_t numMessages)
                                  {
                                      return GetMessagesFromQueue(m_messageQueue,
                                                                  MessageType::STATEFUL,
                                                                  numMessages,
                                                                  [this]() { return m_agentInfo.GetMetadataInfo(); });
                                  },
                                  [this]([[maybe_unused]] const int messageCount, const std::string&)
                                  { PopMessagesFromQueue(m_messageQueue, MessageType::STATEFUL, messageCount); }),
                              "Stateful");

    m_taskManager.EnqueueTask(m_communicator.StatelessMessageProcessingTask(
                                  [this](const size_t numMessages)
                                  {
                                      return GetMessagesFromQueue(m_messageQueue,
                                                                  MessageType::STATELESS,
                                                                  numMessages,
                                                                  [this]() { return m_agentInfo.GetMetadataInfo(); });
                                  },
                                  [this]([[maybe_unused]] const int messageCount, const std::string&)
                                  { PopMessagesFromQueue(m_messageQueue, MessageType::STATELESS, messageCount); }),
                              "Stateless");

    m_moduleManager.AddModules();
    m_moduleManager.Start();

    m_taskManager.EnqueueTask(
        m_commandHandler.CommandsProcessingTask(
            [this]() { return GetCommandFromQueue(m_messageQueue); },
            [this]() { return PopCommandFromQueue(m_messageQueue); },
            [this](const module_command::CommandEntry& cmd) { return ReportCommandResult(cmd, m_messageQueue); },
            [this](module_command::CommandEntry& cmd)
            {
                if (cmd.Module == module_command::CENTRALIZED_CONFIGURATION_MODULE)
                {
                    return DispatchCommand(
                        cmd,
                        [this](std::string command, nlohmann::json parameters) {
                            return m_centralizedConfiguration.ExecuteCommand(std::move(command), std::move(parameters));
                        },
                        m_messageQueue);
                }
                else if (cmd.Module == module_command::RESTART_HANDLER_MODULE)
                {
                    LogInfo("Restart: Initiating restart");
                    return restart_handler::RestartHandler::RestartAgent();
                }
                return DispatchCommand(cmd, m_moduleManager.GetModule(cmd.Module), m_messageQueue);
            }),
        "CommandsProcessing");

    {
        const std::unique_lock<std::mutex> lock(m_reloadMutex);
        m_running.store(true);
    }

    m_signalHandler->WaitForSignal();

    {
        const std::unique_lock<std::mutex> lock(m_reloadMutex);
        m_running.store(false);
    }

    m_commandHandler.Stop();
    m_communicator.Stop();
    m_moduleManager.Stop();
}
