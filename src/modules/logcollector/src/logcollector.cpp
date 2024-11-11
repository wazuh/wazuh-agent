#include <logcollector.hpp>
#include <logger.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>

#include <chrono>
#include <iomanip>
#include <sstream>

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

// NOLINTBEGIN(performance-unnecessary-value-param)
Co_CommandExecutionResult Logcollector::ExecuteCommand(const std::string command,
                                                    [[maybe_unused]] const nlohmann::json parameters) {
  LogInfo("Logcollector command: ", command);
  co_return module_command::CommandExecutionResult{module_command::Status::SUCCESS, "OK"};
}
// NOLINTEND(performance-unnecessary-value-param)

void Logcollector::SetPushMessageFunction(const std::function<int(Message)>& pushMessage) {
    m_pushMessage = pushMessage;
}

void Logcollector::SendMessage(const std::string& location, const std::string& log, const std::string& collectorType) {
    auto metadata = nlohmann::json::object();
    auto data = nlohmann::json::object();


    auto getCurrentTimestamp = []() {
        constexpr int MILLISECS_IN_A_SEC = 1000;
        auto now = std::chrono::system_clock::now();
        auto time_t_now = std::chrono::system_clock::to_time_t(now);
        auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % MILLISECS_IN_A_SEC;

        std::tm tm_now{};
#ifdef _WIN32
        gmtime_s(&tm_now, &time_t_now);  // MSVC (Windows)
#else
        gmtime_r(&time_t_now, &tm_now);  // Linux/macOS (POSIX)
#endif

        // Formatear el timestamp
        std::ostringstream oss;
        oss << std::put_time(&tm_now, "%Y-%m-%dT%H:%M:%S") << '.'
            << std::setw(3) << std::setfill('0') << milliseconds << "Z"; // Usar 'milliseconds' como entero

        return oss.str();
    };

    metadata["module"] = m_moduleName;
    metadata["type"] = collectorType;

    data["log"]["file"]["path"] = location;
    data["tags"] = nlohmann::json::array({"mvp"});
    data["event"]["original"] = log;
    data["event"]["ingested"] = getCurrentTimestamp();
    data["event"]["module"] = m_moduleName;
    data["event"]["provider"] = "syslog";

    auto message = Message(MessageType::STATELESS, data, m_moduleName, collectorType, metadata.dump());
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
