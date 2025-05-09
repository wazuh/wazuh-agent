#include <sca.hpp>

#include <sca_event_handler.hpp>
#include <sca_policy.hpp>
#include <sca_policy_loader.hpp>

#include <config.h>
#include <dbsync.hpp>
#include <filesystem_wrapper.hpp>
#include <logger.hpp>

constexpr auto POLICY_SQL_STATEMENT {
    R"(CREATE TABLE IF NOT EXISTS sca_policy (
    id TEXT PRIMARY KEY,
    name TEXT,
    file TEXT,
    description TEXT,
    refs TEXT);)"};

constexpr auto CHECK_SQL_STATEMENT {
    R"(CREATE TABLE IF NOT EXISTS sca_check (
    id TEXT PRIMARY KEY,
    policy_id TEXT REFERENCES sca_policy(id),
    name TEXT,
    description TEXT,
    rationale TEXT,
    remediation TEXT,
    refs TEXT,
    result TEXT DEFAULT 'Not run',
    reason TEXT,
    condition TEXT,
    compliance TEXT,
    rules TEXT);)"};

SecurityConfigurationAssessment::SecurityConfigurationAssessment(
    std::shared_ptr<const configuration::ConfigurationParser> configurationParser,
    std::string agentUUID,
    std::shared_ptr<IDBSync> dbSync,
    std::shared_ptr<IFileSystemWrapper> fileSystemWrapper)
    : m_agentUUID(std::move(agentUUID))
    , m_dBSync(dbSync ? std::move(dbSync)
                      : std::make_shared<DBSync>(
                            HostType::AGENT,
                            DbEngineType::SQLITE3,
                            configurationParser->GetConfigOrDefault(config::DEFAULT_DATA_PATH, "agent", "path.data") +
                                "/" + SCA_DB_DISK_NAME,
                            GetCreateStatement(),
                            DbManagement::PERSISTENT))
    , m_fileSystemWrapper(fileSystemWrapper ? std::move(fileSystemWrapper)
                                            : std::make_shared<file_system::FileSystemWrapper>())
{
}

void SecurityConfigurationAssessment::Run()
{
    if (!m_enabled)
    {
        LogInfo("SCA module is disabled.");
        return;
    }
    if (!m_ioContext)
    {
        LogError("SCA module doesn't have a valid io context.");
        return;
    }
    LogInfo("SCA module running.");
    m_ioContext->run();
}

void SecurityConfigurationAssessment::Setup(
    std::shared_ptr<const configuration::ConfigurationParser> configurationParser)
{
    m_enabled = configurationParser->GetConfigOrDefault(config::sca::DEFAULT_ENABLED, "sca", "enabled");
    m_scanOnStart = configurationParser->GetConfigOrDefault(config::sca::DEFAULT_SCAN_ON_START, "sca", "scan_on_start");
    m_scanInterval = configurationParser->GetTimeConfigOrDefault(config::sca::DEFAULT_INTERVAL, "sca", "interval");

    m_policies = [this, &configurationParser]()
    {
        const SCAPolicyLoader policyLoader(m_fileSystemWrapper, configurationParser, m_dBSync);
        return policyLoader.LoadPolicies(
            [this](auto policyData, auto checksData)
            {
                const SCAEventHandler eventHandler(m_agentUUID, m_dBSync, m_pushMessage);
                eventHandler.ReportPoliciesDelta(policyData, checksData);
            });
    }();

    m_ioContext = std::make_unique<boost::asio::io_context>();

    for (auto& policy : m_policies)
    {
        EnqueueTask(
            policy->Run(m_scanInterval,
                        m_scanOnStart,
                        [this](const std::string& policyId, const std::string& checkId, const std::string& result)
                        {
                            const SCAEventHandler eventHandler(m_agentUUID, m_dBSync, m_pushMessage);
                            eventHandler.ReportCheckResult(policyId, checkId, result);
                        }));
    }
}

void SecurityConfigurationAssessment::Stop()
{
    if (!m_ioContext)
    {
        LogError("SCA module doesn't have a valid io context.");
        return;
    }
    for (auto& policy : m_policies)
    {
        policy->Stop();
    }
    m_ioContext->stop();
    LogInfo("SCA module stopped.");
}

Co_CommandExecutionResult
SecurityConfigurationAssessment::ExecuteCommand([[maybe_unused]] const std::string command,
                                                [[maybe_unused]] const nlohmann::json parameters)
{
    if (!m_enabled)
    {
        LogInfo("SCA module is disabled.");
        co_return module_command::CommandExecutionResult {module_command::Status::FAILURE, "Module is disabled"};
    }

    LogInfo("Command: {}", command);
    co_return module_command::CommandExecutionResult {module_command::Status::SUCCESS, "Command not implemented yet"};
}

const std::string& SecurityConfigurationAssessment::Name() const
{
    return m_name;
}

void SecurityConfigurationAssessment::SetPushMessageFunction(const std::function<int(Message)>& pushMessage)
{
    m_pushMessage = pushMessage;
}

void SecurityConfigurationAssessment::EnqueueTask(boost::asio::awaitable<void> task)
{
    if (!m_ioContext)
    {
        LogError("SCA module doesn't have a valid io context.");
        return;
    }

    // NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    boost::asio::co_spawn(
        *m_ioContext,
        [task = std::move(task)]() mutable -> boost::asio::awaitable<void>
        {
            try
            {
                co_await std::move(task);
            }
            catch (const std::exception& e)
            {
                LogError("SCA coroutine task exited with an exception: {}", e.what());
            }
        },
        boost::asio::detached);
    // NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)
}

std::string SecurityConfigurationAssessment::GetCreateStatement() const
{
    std::string ret;
    ret += POLICY_SQL_STATEMENT;
    ret += CHECK_SQL_STATEMENT;

    return ret;
}
