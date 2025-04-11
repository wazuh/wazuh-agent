#include <sca.hpp>

#include <config.h>
#include <dbsync.hpp>
#include <sca_event_handler.hpp>
#include <sca_policy_loader.hpp>

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
    result TEXT,
    reason TEXT,
    condition TEXT,
    compliance TEXT,
    rules TEXT);)"};

SecurityConfigurationAssessment::SecurityConfigurationAssessment(
    std::shared_ptr<const configuration::ConfigurationParser> configurationParser,
    std::shared_ptr<IDBSync> dbSync,
    std::shared_ptr<IFileSystemWrapper> fileSystemWrapper)
    : m_dBSync(dbSync ? std::move(dbSync)
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
    Setup(configurationParser);
}

void SecurityConfigurationAssessment::Run()
{
    // Execute the policies (run io context)
    // Each policy should:
    // Run regex engine, check type of policies
    // Create a report and send it to the server
    m_ioContext.run();
}

void SecurityConfigurationAssessment::Setup(
    std::shared_ptr<const configuration::ConfigurationParser> configurationParser)
{
    m_enabled = configurationParser->GetConfigOrDefault(config::sca::DEFAULT_ENABLED, "sca", "enabled");
    m_scanOnStart = configurationParser->GetConfigOrDefault(config::sca::DEFAULT_SCAN_ON_START, "sca", "scan_on_start");
    m_scanInterval = configurationParser->GetTimeConfigOrDefault(config::sca::DEFAULT_INTERVAL, "sca", "interval");

    m_policies = [this, &configurationParser]()
    {
        const SCAPolicyLoader policyLoader(m_fileSystemWrapper, configurationParser, m_pushMessage, m_dBSync);
        return policyLoader.GetPolicies();
    }();
}

void SecurityConfigurationAssessment::Stop()
{
    // Stop the policies
    // Stop the regex engine
    m_ioContext.stop();
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
    // NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    boost::asio::co_spawn(
        m_ioContext,
        [task = std::move(task)]() mutable -> boost::asio::awaitable<void>
        {
            try
            {
                co_await std::move(task);
            }
            catch (const std::exception& e)
            {
                LogError("Logcollector coroutine task exited with an exception: {}", e.what());
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
