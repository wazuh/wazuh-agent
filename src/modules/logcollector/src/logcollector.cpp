#include <logcollector.hpp>
#include <logger.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>

#include "file_reader.hpp"

using namespace logcollector;

void Logcollector::Start() {
    if (!m_enabled) {
        LogInfo("Logcollector is disabled");
        return;
    }

    LogInfo("Logcollector started");
    m_ioContext.run();
}

void Logcollector::EnqueueTask(boost::asio::awaitable<void> task) {
    boost::asio::co_spawn(m_ioContext, std::move(task), boost::asio::detached);
}

void Logcollector::Setup(const configuration::ConfigurationParser& configurationParser) {
    try {
        m_enabled = configurationParser.GetConfig<bool>("logcollector", "enabled");
    } catch (std::exception&) {
        m_enabled = true;
    }

    SetupFileReader(configurationParser);
}

void Logcollector::SetupFileReader(const configuration::ConfigurationParser& configurationParser) {
    long fileWait = config::DEFAULT_FILE_WAIT;
    long reloadInterval = config::DEFAULT_RELOAD_INTERVAL;

    try {
        fileWait = configurationParser.GetConfig<long>("logcollector", "file_wait");
    } catch (std::exception&) {
        fileWait = config::DEFAULT_FILE_WAIT;
    }

    try {
        reloadInterval = configurationParser.GetConfig<long>("logcollector", "reload_interval");
    } catch (std::exception&) {
        reloadInterval = config::DEFAULT_FILE_WAIT;
    }

    try {
        auto localfiles = configurationParser.GetConfig<std::vector<std::string>>("logcollector", "localfiles");

        for (auto& lf : localfiles) {
            AddReader(std::make_shared<FileReader>(*this, lf, fileWait, reloadInterval));
        }
    } catch (std::exception&) {
        LogTrace("No localfiles defined");
    }
}

void Logcollector::Stop() {
    m_ioContext.stop();
    LogInfo("Logcollector stopped");
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
Co_CommandExecutionResult Logcollector::ExecuteCommand(const std::string command, [[maybe_unused]] const nlohmann::json parameters) {
    LogInfo("Logcollector command: ", command);
    co_return module_command::CommandExecutionResult{module_command::Status::SUCCESS, "OK"};
}

void Logcollector::SetPushMessageFunction(const std::function<int(Message)>& pushMessage) {
    m_pushMessage = pushMessage;
}

void Logcollector::SendMessage(const std::string& location, const std::string& log) {
    auto data = nlohmann::json::object();
    auto event = nlohmann::json::object();

    data["location"] = location;
    data["event"] = event;
    event["original"] = log;

    auto message = Message(MessageType::STATELESS, data, m_moduleName);
    m_pushMessage(message);

    LogTrace("Message pushed: '{}':'{}'", location, log);
}

void Logcollector::AddReader(std::shared_ptr<IReader> reader) {
    m_readers.push_back(reader);
    EnqueueTask(reader->Run());
}

Awaitable Logcollector::Wait(std::chrono::milliseconds ms) {
    co_await boost::asio::steady_timer(m_ioContext, ms).async_wait(boost::asio::use_awaitable);
}

Awaitable Logcollector::Wait(std::chrono::seconds sec) {
    co_await boost::asio::steady_timer(m_ioContext, sec).async_wait(boost::asio::use_awaitable);
}
