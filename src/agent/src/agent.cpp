#include <agent.hpp>

#include <command_handler_utils.hpp>
#include <config.h>
#include <http_client.hpp>
#include <message.hpp>
#include <message_queue_utils.hpp>
#include <module_command/command_entry.hpp>

#include <filesystem>
#include <memory>

Agent::Agent(const std::string& configFilePath, std::unique_ptr<ISignalHandler> signalHandler)
    : m_configurationParser(configFilePath.empty() ? std::make_shared<configuration::ConfigurationParser>()
                                                   : std::make_shared<configuration::ConfigurationParser>(
                                                         std::filesystem::path(configFilePath)))
    , m_dataPath(
          m_configurationParser->GetConfig<std::string>("agent", "path.data").value_or(config::DEFAULT_DATA_PATH))
    , m_messageQueue(std::make_shared<MultiTypeQueue>(
          [this]<typename T>(std::string table, std::string key) -> std::optional<T>
          { return m_configurationParser->GetConfig<T>(std::move(table), std::move(key)); }))
    , m_signalHandler(std::move(signalHandler))
    , m_agentInfo(
          m_dataPath, [this]() { return m_sysInfo.os(); }, [this]() { return m_sysInfo.networks(); })
    , m_communicator(
          std::make_unique<http_client::HttpClient>(),
          m_agentInfo.GetUUID(),
          m_agentInfo.GetKey(),
          [this]() { return m_agentInfo.GetHeaderInfo(); },
          [this]<typename T>(std::string table, std::string key) -> std::optional<T>
          { return m_configurationParser->GetConfig<T>(std::move(table), std::move(key)); })
    , m_moduleManager([this](Message message) -> int { return m_messageQueue->push(std::move(message)); },
                      m_configurationParser,
                      m_agentInfo.GetUUID())
    , m_commandHandler(m_dataPath)
{
    // Check if agent is registered
    if (m_agentInfo.GetName().empty() || m_agentInfo.GetKey().empty() || m_agentInfo.GetUUID().empty())
    {
        throw std::runtime_error("The agent is not registered");
    }

    m_configurationParser->SetGetGroupIdsFunction([this]() { return m_agentInfo.GetGroups(); });

    m_centralizedConfiguration.SetGroupIdFunction(
        [this](const std::vector<std::string>& groups)
        {
            m_agentInfo.SetGroups(groups);
            return m_agentInfo.SaveGroups();
        });

    m_centralizedConfiguration.GetGroupIdFunction([this]() { return m_agentInfo.GetGroups(); });

    // NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    m_centralizedConfiguration.SetDownloadGroupFilesFunction(
        [this](std::string groupId, std::string destinationPath) -> boost::asio::awaitable<bool>
        {
            co_return co_await m_communicator.GetGroupConfigurationFromManager(std::move(groupId),
                                                                               std::move(destinationPath));
        });
    // NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)

    m_centralizedConfiguration.ValidateFileFunction([this](const std::filesystem::path& fileToValidate)
                                                    { return m_configurationParser->isValidYamlFile(fileToValidate); });

    m_centralizedConfiguration.ReloadModulesFunction([this]() { ReloadModules(); });

    m_agentThreadCount =
        m_configurationParser->GetConfig<size_t>("agent", "thread_count").value_or(config::DEFAULT_THREAD_COUNT);

    if (m_agentThreadCount < config::DEFAULT_THREAD_COUNT)
    {
        LogWarn("thread_count must be greater than {}. Using default value.", config::DEFAULT_THREAD_COUNT);
        m_agentThreadCount = config::DEFAULT_THREAD_COUNT;
    }
}

Agent::~Agent()
{
    m_taskManager.Stop();
}

void Agent::ReloadModules()
{
    std::lock_guard<std::mutex> lock(m_reloadMutex);

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
                                                                  [this]()
                                                                  { return m_agentInfo.GetMetadataInfo(false); });
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
                                                                  [this]()
                                                                  { return m_agentInfo.GetMetadataInfo(false); });
                                  },
                                  [this]([[maybe_unused]] const int messageCount, const std::string&)
                                  { PopMessagesFromQueue(m_messageQueue, MessageType::STATELESS, messageCount); }),
                              "Stateless");

    m_moduleManager.AddModules();
    m_moduleManager.Start();

    m_taskManager.EnqueueTask(
        m_commandHandler.CommandsProcessingTask<module_command::CommandEntry>(
            [this]() { return GetCommandFromQueue(m_messageQueue); },
            [this]() { return PopCommandFromQueue(m_messageQueue); },
            [this](const module_command::CommandEntry& cmd) { return ReportCommandResult(cmd, m_messageQueue); },
            [this](module_command::CommandEntry& cmd)
            {
                if (cmd.Module == "CentralizedConfiguration")
                {
                    return DispatchCommand(
                        cmd,
                        [this](std::string command, nlohmann::json parameters) {
                            return m_centralizedConfiguration.ExecuteCommand(std::move(command), std::move(parameters));
                        },
                        m_messageQueue);
                }
                return DispatchCommand(cmd, m_moduleManager.GetModule(cmd.Module), m_messageQueue);
            }),
        "CommandsProcessing");

    {
        std::unique_lock<std::mutex> lock(m_reloadMutex);
        m_running.store(true);
    }

    m_signalHandler->WaitForSignal();

    {
        std::unique_lock<std::mutex> lock(m_reloadMutex);
        m_running.store(false);
    }

    m_commandHandler.Stop();
    m_communicator.Stop();
    m_moduleManager.Stop();
}
