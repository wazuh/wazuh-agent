#include <logcollector.hpp>
#include <logger.hpp>

void Logcollector::Start() const {
    if (!m_enabled) {
        LogInfo("Logcollector is disabled");
        return;
    }

    LogInfo("Logcollector started");

    auto data = nlohmann::json::object();
    data["payload"] = "Hello World";
    auto message = Message(MessageType::STATELESS, data, m_moduleName);
    m_messageQueue->push(message);
}

void Logcollector::Setup(const configuration::ConfigurationParser& configurationParser) {
    m_enabled = configurationParser.GetConfig<bool>("inventory", "enabled");
}

void Logcollector::Stop() {
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
