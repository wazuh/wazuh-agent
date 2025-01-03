#include <logcollector.hpp>

#include <logger.hpp>
#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <config.h>
#include <timeHelper.h>

#include <chrono>
#include <iomanip>
#include <map>
#include <sstream>

#include "file_reader.hpp"

#ifdef _WIN32
#include "we_reader_win.hpp"
#endif
using namespace logcollector;


void Logcollector::Start()
{
    if (!m_enabled) {
        LogInfo("Logcollector module is disabled.");
        return;
    }

    LogInfo("Logcollector module started.");
    m_ioContext.run();
}

void Logcollector::EnqueueTask(boost::asio::awaitable<void> task)
{
    boost::asio::co_spawn(m_ioContext, std::move(task), boost::asio::detached);
}

void Logcollector::Setup(std::shared_ptr<const configuration::ConfigurationParser> configurationParser)
{
    if (!configurationParser) {
        LogError("Invalid Configuration Parser passed to setup, module set to disabled.");
        m_enabled = false;
        return;
    }

    m_enabled = configurationParser->GetConfig<bool>("logcollector", "enabled").value_or(config::logcollector::DEFAULT_ENABLED);

    if (m_ioContext.stopped())
    {
        m_ioContext.restart();
    }

    SetupFileReader(configurationParser);
#ifdef _WIN32
    SetupWEReader(configurationParser);
#endif
}

void Logcollector::SetupFileReader(const std::shared_ptr<const configuration::ConfigurationParser> configurationParser)
{
    auto fileWait = configurationParser->GetConfig<std::time_t>("logcollector", "file_wait").value_or(config::logcollector::DEFAULT_FILE_WAIT);

    auto reloadInterval = configurationParser->GetConfig<std::time_t>("logcollector", "reload_interval").value_or(config::logcollector::DEFAULT_RELOAD_INTERVAL);

    auto localfiles = configurationParser->GetConfig<std::vector<std::string>>("logcollector", "localfiles").value_or(std::vector<std::string>({config::logcollector::DEFAULT_LOCALFILES}));

    for (auto& lf : localfiles) {
        AddReader(std::make_shared<FileReader>(*this, lf, fileWait, reloadInterval));
    }
}

#ifdef _WIN32
void Logcollector::SetupWEReader(const std::shared_ptr<const configuration::ConfigurationParser> configurationParser)
{
    const auto refreshInterval = configurationParser->GetConfig<time_t>("logcollector", "channel_refresh").value_or(config::logcollector::CHANNEL_REFRESH_INTERVAL);

    const auto bookmarkEnabled = configurationParser->GetConfig<bool>("logcollector", "use_bookmark").value_or(config::logcollector::DEFAULT_USE_BOOKMARK);

    const auto windowsConfig = configurationParser->GetConfig<std::vector<std::map<std::string, std::string>>>("logcollector", "windows").value_or(
        std::vector<std::map<std::string, std::string>> {});

    std::vector<std::string> channelsList;
    std::vector<std::string> queriesList;
    for (auto& entry : windowsConfig)
    {
        auto channel = entry.at("channel");
        auto query = entry.at("query");
        channelsList.emplace_back(channel);
        queriesList.emplace_back(query);
    }
    AddReader(std::make_shared<WindowsEventTracerReader>(*this, channelsList, queriesList, refreshInterval, bookmarkEnabled));
}
#endif

void Logcollector::Stop()
{
    m_ioContext.stop();
    LogInfo("Logcollector module stopped.");
}

// NOLINTBEGIN(performance-unnecessary-value-param)
Co_CommandExecutionResult Logcollector::ExecuteCommand(const std::string command,
                                                    [[maybe_unused]] const nlohmann::json parameters)
                                                    {
  LogInfo("Logcollector command: ", command);
  co_return module_command::CommandExecutionResult{module_command::Status::SUCCESS, "OK"};
}
// NOLINTEND(performance-unnecessary-value-param)

void Logcollector::SetPushMessageFunction(const std::function<int(Message)>& pushMessage) {
    m_pushMessage = pushMessage;
}

void Logcollector::SendMessage(const std::string& location, const std::string& log, const std::string& collectorType)
{
    auto metadata = nlohmann::json::object();
    auto data = nlohmann::json::object();

    metadata["module"] = m_moduleName;
    metadata["type"] = collectorType;

    data["log"]["file"]["path"] = location;
    data["tags"] = nlohmann::json::array({"mvp"});
    data["event"]["original"] = log;
    data["event"]["created"] = Utils::getCurrentISO8601();
    data["event"]["module"] = m_moduleName;
    data["event"]["provider"] = "syslog";

    auto message = Message(MessageType::STATELESS, data, m_moduleName, collectorType, metadata.dump());
    m_pushMessage(message);

    //TODO: undo
    LogInfo("Message pushed: '{}':'{}'", location, log);
}

void Logcollector::AddReader(std::shared_ptr<IReader> reader)
{
    //TODO: do we need m_readers ?
    m_readers.push_back(reader);
    EnqueueTask(reader->Run());
}

Awaitable Logcollector::Wait(std::chrono::milliseconds ms)
{
    co_await boost::asio::steady_timer(m_ioContext, ms).async_wait(boost::asio::use_awaitable);
}
