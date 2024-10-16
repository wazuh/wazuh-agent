#include <logcollector.hpp>
#include <logger.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>

#include "file_reader.hpp"

using namespace logcollector;
using namespace std;

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
    m_enabled = configurationParser.GetConfig<bool>("logcollector", "enabled");
    auto fileReader = make_shared<FileReader>(*this, "/var/log/syslog");
    AddReader(fileReader);
    AddReader(make_shared<FileReader>(*this, "/root/test/*.log"));
}

void Logcollector::Stop() {
    m_ioContext.stop();
    LogInfo("Logcollector stopped");
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
Co_CommandExecutionResult Logcollector::ExecuteCommand(const std::string query) {
    LogInfo("Logcollector query: ", query);
    co_return module_command::CommandExecutionResult{module_command::Status::SUCCESS, "OK"};
}

void Logcollector::SetMessageQueue(const std::shared_ptr<IMultiTypeQueue> queue) {
    m_messageQueue = queue;
}

void Logcollector::SendMessage(const std::string& location, const std::string& log) {
    auto data = nlohmann::json::object();
    auto event = nlohmann::json::object();

    data["location"] = location;
    data["event"] = event;
    event["original"] = log;

    auto message = Message(MessageType::STATELESS, data, m_moduleName);
    m_messageQueue->push(message);

    LogTrace("Message pushed: '{}':'{}'", location, log);
}

void Logcollector::AddReader(shared_ptr<IReader> reader) {
    m_readers.push_back(reader);
    EnqueueTask(reader->Run());
}

Awaitable Logcollector::Wait(chrono::milliseconds ms) {
    co_await boost::asio::steady_timer(m_ioContext, ms).async_wait(boost::asio::use_awaitable);
}

Awaitable Logcollector::Wait(chrono::seconds sec) {
    co_await boost::asio::steady_timer(m_ioContext, sec).async_wait(boost::asio::use_awaitable);
}
