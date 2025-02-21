#include <logcollector.hpp>

#include <boost/asio/co_spawn.hpp>
#include <boost/asio/detached.hpp>
#include <boost/asio/redirect_error.hpp>
#include <config.h>
#include <logger.hpp>
#include <timeHelper.h>

#include <chrono>
#include <iomanip>
#include <map>
#include <sstream>

#include "file_reader.hpp"

using namespace logcollector;

namespace logcollector
{
    constexpr int ACTIVE_READERS_WAIT_MS = 10;
}

void Logcollector::Start()
{
    if (!m_enabled)
    {
        LogInfo("Logcollector module is disabled.");
        return;
    }

    LogInfo("Logcollector module started.");
    m_ioContext.run();
}

void Logcollector::EnqueueTask(boost::asio::awaitable<void> task)
{
    // NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    boost::asio::co_spawn(
        m_ioContext,
        [task = std::move(task), this]() mutable -> boost::asio::awaitable<void>
        {
            try
            {
                m_activeReaders++;
                co_await std::move(task);
            }
            catch (const std::exception& e)
            {
                LogError("Logcollector coroutine task exited with an exception: {}", e.what());
            }
            m_activeReaders--;
        },
        boost::asio::detached);
    // NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)
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

    if (m_ioContext.stopped())
    {
        m_ioContext.restart();
    }

    SetupFileReader(configurationParser);
    AddPlatformSpecificReader(configurationParser);
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
        AddReader(std::make_shared<FileReader>(*this, lf, fileWait, reloadInterval));
    }
}

void Logcollector::Stop()
{
    CleanAllReaders();
    m_ioContext.stop();
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
    else if (m_ioContext.stopped())
    {
        LogInfo("Logcollector module is stopped.");
        co_return module_command::CommandExecutionResult {module_command::Status::FAILURE, "Module is stopped"};
    }
    LogInfo("Logcollector command: ", command);
    co_return module_command::CommandExecutionResult {module_command::Status::SUCCESS, "Command not implemented yet"};
}

// NOLINTEND(performance-unnecessary-value-param)

void Logcollector::SetPushMessageFunction(const std::function<int(Message)>& pushMessage)
{
    m_pushMessage = pushMessage;
}

void Logcollector::SendMessage(const std::string& location, const std::string& log, const std::string& collectorType)
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
    if (!m_ioContext.stopped())
    {
        auto timer = boost::asio::steady_timer(m_ioContext, ms);
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
