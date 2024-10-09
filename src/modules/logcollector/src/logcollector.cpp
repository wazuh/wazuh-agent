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

    auto fileReader = FileReader();
    boost::asio::co_spawn(m_ioContext, fileReader.run(), boost::asio::detached);

    m_ioContext.run();
}

void Logcollector::EnqueueTask(boost::asio::awaitable<void> task) {
    boost::asio::co_spawn(m_ioContext, std::move(task), boost::asio::detached);
}

void Logcollector::Setup(const configuration::ConfigurationParser& configurationParser) {
    m_enabled = configurationParser.GetConfig<bool>("logcollector", "enabled");
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

    data["location"] = location;
    data["event_original"] = log;

    auto message = Message(MessageType::STATELESS, data, m_moduleName);
    m_messageQueue->push(message);

    LogTrace("Message pushed: '{}':'{}'", location, log);
}
