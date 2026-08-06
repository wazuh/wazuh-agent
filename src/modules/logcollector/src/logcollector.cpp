#include <logcollector.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/ip/address.hpp>
#include <boost/asio/redirect_error.hpp>
#include <config.h>
#include <logger.hpp>
#include <timeHelper.hpp>

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <iomanip>
#include <map>
#include <set>
#include <sstream>

#include "file_reader.hpp"
#include "syslog_reader.hpp"

using namespace logcollector;

namespace logcollector
{
    constexpr int ACTIVE_READERS_WAIT_MS = 10;
}

void Logcollector::Run()
{
    if (!m_enabled)
    {
        LogInfo("Logcollector module is disabled.");
        return;
    }

    LogInfo("Logcollector module running.");
    m_taskManager.RunSingleThread();
}

boost::asio::awaitable<void> Logcollector::WrapWithCounter(boost::asio::awaitable<void> task)
{
    ++m_activeReaders;
    try
    {
        co_await std::move(task);
    }
    catch (...)
    {
        LogError("Logcollector coroutine task exited with an exception.");
    }
    --m_activeReaders;
    co_return;
}

void Logcollector::EnqueueTask(boost::asio::awaitable<void> task)
{
    m_taskManager.EnqueueTask(WrapWithCounter(std::move(task)));
}

void Logcollector::Setup(std::shared_ptr<const configuration::ConfigurationParser> configurationParser)
{
    if (!configurationParser)
    {
        LogError("Invalid Configuration Parser passed to setup, module set to disabled.");
        m_enabled = false;
        return;
    }

    m_enabled =
        configurationParser->GetConfigOrDefault(config::logcollector::DEFAULT_ENABLED, "logcollector", "enabled");

    SetupFileReader(configurationParser);
    SetupSyslogReaders(configurationParser);
    AddPlatformSpecificReader(configurationParser);
}

void Logcollector::SetupSyslogReaders(
    const std::shared_ptr<const configuration::ConfigurationParser> configurationParser)
{
    const auto syslogConfigs = configurationParser->GetConfigOrDefault<YAML::Node>(
        YAML::Node(YAML::NodeType::Sequence), "logcollector", "syslog");

    constexpr int MIN_PORT = 1;
    constexpr int MAX_PORT = 65535;

    std::set<std::string> seenListeners;

    for (const auto& config : syslogConfigs)
    {
        if (!config.IsMap())
        {
            LogWarn("Invalid agent-side syslog listener configuration: entry is not a mapping.");
            continue;
        }

        auto protocolStr = config["protocol"].as<std::string>("");
        std::transform(protocolStr.begin(),
                       protocolStr.end(),
                       protocolStr.begin(),
                       [](unsigned char c) { return static_cast<char>(std::tolower(c)); });

        SyslogProtocol protocol = SyslogProtocol::Udp;
        if (protocolStr == "udp")
        {
            protocol = SyslogProtocol::Udp;
        }
        else if (protocolStr == "tcp")
        {
            protocol = SyslogProtocol::Tcp;
        }
        else
        {
            LogError("Invalid agent-side syslog listener configuration: unsupported protocol {}.",
                     protocolStr.empty() ? "(missing)" : protocolStr);
            continue;
        }

        if (!config["port"])
        {
            LogError("Invalid agent-side syslog listener configuration: missing port.");
            continue;
        }

        int port = 0;
        try
        {
            port = config["port"].as<int>();
        }
        catch (const std::exception&)
        {
            LogError("Invalid agent-side syslog listener configuration: invalid port {}.",
                     config["port"].as<std::string>(""));
            continue;
        }

        if (port < MIN_PORT || port > MAX_PORT)
        {
            LogError("Invalid agent-side syslog listener configuration: invalid port {}.", port);
            continue;
        }

        const auto bindAddress = config["bind_address"].as<std::string>("127.0.0.1");

        boost::system::error_code ec;
        boost::asio::ip::make_address(bindAddress, ec);
        if (ec)
        {
            LogError("Invalid agent-side syslog listener configuration: invalid bind address {}.", bindAddress);
            continue;
        }

        const auto listenerId =
            SyslogReader::ProtocolToString(protocol) + ":" + bindAddress + ":" + std::to_string(port);

        if (!seenListeners.insert(listenerId).second)
        {
            LogError("Invalid agent-side syslog listener configuration: duplicate listener {}.", listenerId);
            continue;
        }

        AddReader(std::make_shared<SyslogReader>(
            [this](const std::string& location, const std::string& log, const std::string& collectorType)
            { PushMessage(location, log, collectorType); },
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines)
            [this](std::chrono::milliseconds duration) -> Awaitable { co_await Wait(duration); },
            [this](Awaitable task) { EnqueueTask(std::move(task)); },
            protocol,
            bindAddress,
            static_cast<std::uint16_t>(port)));
    }
}

void Logcollector::SetupFileReader(const std::shared_ptr<const configuration::ConfigurationParser> configurationParser)
{
    const auto fileWait = configurationParser->GetTimeConfigOrDefault(
        config::logcollector::DEFAULT_FILE_WAIT, "logcollector", "read_interval");

    const auto reloadInterval = configurationParser->GetTimeConfigOrDefault(
        config::logcollector::DEFAULT_RELOAD_INTERVAL, "logcollector", "reload_interval");

    const auto localFilesDefault = std::vector<std::string> {config::logcollector::DEFAULT_LOCALFILES};

    const auto localfiles = configurationParser->GetConfigOrDefault(localFilesDefault, "logcollector", "localfiles");

    for (const auto& lf : localfiles)
    {
        AddReader(std::make_shared<FileReader>(
            [this](const std::string& location, const std::string& log, const std::string& collectorType)
            { PushMessage(location, log, collectorType); },
            // NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines)
            [this](std::chrono::milliseconds duration) -> Awaitable { co_await Wait(duration); },
            [this](Awaitable task) { EnqueueTask(std::move(task)); },
            lf,
            fileWait,
            reloadInterval));
    }
}

void Logcollector::Stop()
{
    CleanAllReaders();
    m_taskManager.Stop();
    LogInfo("Logcollector module stopped.");
}

// NOLINTBEGIN(performance-unnecessary-value-param)
Co_CommandExecutionResult Logcollector::ExecuteCommand(const std::string command,
                                                       [[maybe_unused]] const nlohmann::json parameters)
{
    if (!m_enabled)
    {
        LogInfo("Logcollector module is disabled.");
        co_return module_command::CommandExecutionResult {module_command::Status::FAILURE, "Module is disabled"};
    }
    else if (m_taskManager.IsStopped())
    {
        LogInfo("Logcollector module is stopped.");
        co_return module_command::CommandExecutionResult {module_command::Status::FAILURE, "Module is stopped"};
    }
    LogInfo("Logcollector command: ", command);
    co_return module_command::CommandExecutionResult {module_command::Status::SUCCESS, "Command not implemented yet"};
}

// NOLINTEND(performance-unnecessary-value-param)

const std::string& Logcollector::Name() const
{
    return m_moduleName;
}

void Logcollector::SetPushMessageFunction(const std::function<int(Message)>& pushMessage)
{
    m_pushMessage = pushMessage;
}

void Logcollector::PushMessage(const std::string& location, const std::string& log, const std::string& collectorType)
{
    if (!m_pushMessage)
    {
        throw std::runtime_error("Message queue not set, cannot send message.");
    }

    auto metadata = nlohmann::json::object();
    auto data = nlohmann::json::object();

    metadata["module"] = m_moduleName;
    metadata["collector"] = collectorType;

    if (collectorType == FILE_READER_TYPE)
    {
        data["log"]["file"]["path"] = location;
    }
    else
    {
        data["event"]["provider"] = location;
    }
    data["event"]["original"] = log;
    data["event"]["created"] = Utils::getCurrentISO8601();

    auto message = Message(MessageType::STATELESS, data, m_moduleName, collectorType, metadata.dump());
    m_pushMessage(message);

    LogTrace("Message pushed: '{}':'{}'", location, log);
}

void Logcollector::AddReader(std::shared_ptr<IReader> reader)
{
    m_readers.push_back(reader);
    EnqueueTask(reader->Run());
}

void Logcollector::CleanAllReaders()
{
    for (const auto& reader : m_readers)
    {
        reader->Stop();
    }

    {
        const std::lock_guard<std::mutex> lock(m_timersMutex);
        for (const auto& timer : m_timers)
        {
            timer->cancel();
        }
    }

    while (m_activeReaders)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(ACTIVE_READERS_WAIT_MS));
    }
    m_readers.clear();
}

Awaitable Logcollector::Wait(std::chrono::milliseconds ms)
{
    if (!m_taskManager.IsStopped())
    {
        auto timer = m_taskManager.CreateSteadyTimer(ms);

        {
            std::lock_guard<std::mutex> lock(m_timersMutex);
            m_timers.push_back(&timer);
        }

        boost::system::error_code ec;
        co_await timer.async_wait(boost::asio::redirect_error(boost::asio::use_awaitable, ec));

        if (ec)
        {
            if (ec == boost::asio::error::operation_aborted)
            {
                LogDebug("Logcollector coroutine timer was canceled.");
            }
            else
            {
                LogDebug("Logcollector coroutine timer wait failed: {}.", ec.message());
            }
        }

        {
            std::lock_guard<std::mutex> lock(m_timersMutex);
            m_timers.remove(&timer);
        }
    }
}
