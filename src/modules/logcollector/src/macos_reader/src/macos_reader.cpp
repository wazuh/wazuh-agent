#include <macos_reader.hpp>

#include <boost/asio.hpp>
#include <boost/asio/steady_timer.hpp>
#include <fmt/format.h>
#include <fmt/ranges.h>

#include <exception>

namespace
{
    const std::string COLLECTOR_TYPE = "macos";
}

namespace logcollector
{

MacOSReader::MacOSReader(
    Logcollector& logcollector,
    const std::time_t waitInMillis,
    const std::string& logLevel,
    const std::string& query,
    const std::vector<std::string>& logTypes
)
: IReader(logcollector)
, m_osLogStoreWrapper(std::make_unique<OSLogStoreWrapper>())
, m_logLevel(OSLogStoreWrapper::LogLevel::Undefined)
, m_lastLogEntryTimeInSecondsSince1970(std::chrono::duration<double>(std::chrono::system_clock::now().time_since_epoch()).count())
, m_waitInMillis(waitInMillis)
{
    SetLogLevel(logLevel);
    SetQuery(query, logTypes);
}

MacOSReader::MacOSReader(
    std::unique_ptr<IOSLogStoreWrapper> osLogStoreWrapper,
    Logcollector& logcollector,
    const std::time_t waitInMillis,
    const std::string& logLevel,
    const std::string& query,
    const std::vector<std::string>& logTypes
)
: MacOSReader(logcollector, waitInMillis, logLevel, query, logTypes)
{
    m_osLogStoreWrapper = std::move(osLogStoreWrapper);
}

Awaitable MacOSReader::Run()
{
    if (!m_osLogStoreWrapper)
    {
        throw std::runtime_error("OSLogStoreWrapper is not initialized");
    }

    while (m_keepRunning.load())
    {
        const auto logEntries = m_osLogStoreWrapper->AllEntries(m_lastLogEntryTimeInSecondsSince1970, m_query, m_logLevel);

        for (const auto& log : logEntries)
        {
            constexpr double increment = 0.000001;
            const auto slightlyBigger = log.dateInSeconds + increment;
            m_lastLogEntryTimeInSecondsSince1970 = slightlyBigger;

            const auto logAndDate = log.date + " " + log.log;
            m_logcollector.SendMessage(COLLECTOR_TYPE, logAndDate, COLLECTOR_TYPE);
        }

        co_await m_logcollector.Wait(std::chrono::milliseconds(m_waitInMillis));
    }
}

void MacOSReader::Stop()
{
    m_keepRunning.store(false);
}

void MacOSReader::SetLogLevel(const std::string& logLevel)
{
    if (logLevel == "debug")
    {
        m_logLevel = OSLogStoreWrapper::LogLevel::Debug;
    }
    else if (logLevel == "info")
    {
        m_logLevel = OSLogStoreWrapper::LogLevel::Info;
    }
    else if (logLevel == "notice")
    {
        m_logLevel = OSLogStoreWrapper::LogLevel::Notice;
    }
    else if (logLevel == "error")
    {
        m_logLevel = OSLogStoreWrapper::LogLevel::Error;
    }
    else if (logLevel == "fault")
    {
        m_logLevel = OSLogStoreWrapper::LogLevel::Fault;
    }
    else
    {
        m_logLevel = OSLogStoreWrapper::LogLevel::Undefined;
    }
}

void MacOSReader::SetQuery(const std::string& query, const std::vector<std::string>& logTypes)
{
    const auto logTypesPredicate = [&logTypes] () -> std::string
    {
        if (logTypes.empty())
        {
            return "";
        }

        std::vector<std::string> predicates;

        for (const auto& type : logTypes)
        {
            if (type == "trace")
            {
                predicates.emplace_back("eventType == traceEvent");
            }
            else if (type == "log")
            {
                predicates.emplace_back("eventType == logEvent");
            }
            else if (type == "activity")
            {
                predicates.emplace_back("eventType == activityCreateEvent OR eventType == activityTransitionEvent OR eventType == userActionEvent");
            }
        }
        return fmt::format("{}", fmt::join(predicates, " OR "));
    }();

    if (query.empty())
    {
        m_query = fmt::format("{}", logTypesPredicate);
    }
    else if (logTypesPredicate.empty())
    {
        m_query = fmt::format("{}", query);
    }
    else
    {
        m_query = fmt::format("({}) AND ({})", query, logTypesPredicate);
    }
}

} // namespace logcollector
