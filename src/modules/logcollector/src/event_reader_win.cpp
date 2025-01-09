#include "event_reader_win.hpp"

#include <sstream>
#include <iomanip>

#include <logger.hpp>
#include <logcollector.hpp>

namespace logcollector
{

void AddPlatformSpecificReader(std::shared_ptr<const configuration::ConfigurationParser> configurationParser, Logcollector &logcollector)
{
    const auto refreshInterval = configurationParser->GetConfig<time_t>("logcollector", "channel_refresh").value_or(config::logcollector::CHANNEL_REFRESH_INTERVAL);

    const auto windowsConfig = configurationParser->GetConfig<std::vector<std::map<std::string, std::string>>>("logcollector", "windows").value_or(
        std::vector<std::map<std::string, std::string>> {});

    for (auto& entry : windowsConfig)
    {
        auto channel = entry.at("channel");
        auto query = entry.at("query");
        logcollector.AddReader(std::make_shared<WindowsEventTracerReader>(logcollector, channel, query, refreshInterval));
    }
}

WindowsEventTracerReader::WindowsEventTracerReader(Logcollector &logcollector,
                           const std::string channel,
                           const std::string query,
                           const std::time_t channelRefreshInterval) :
    IReader(logcollector),
    m_channel(channel),
    m_query(query),
    m_ChannelsRefreshInterval(channelRefreshInterval) { }

Awaitable WindowsEventTracerReader::Run()
{
    m_logcollector.EnqueueTask(QueryEvents(m_channel, m_query));
    co_return;
}

void WindowsEventTracerReader::Stop()
{
    m_keepRunning.store(false);
}

Awaitable WindowsEventTracerReader::QueryEvents(const std::string channel, const std::string query)
{
    std::wstring wideStringChannel = std::wstring(channel.begin(), channel.end());
    std::wstring wideStringQuery = std::wstring(query.begin(), query.end());

    auto subscriptionCallback = [](EVT_SUBSCRIBE_NOTIFY_ACTION action, PVOID userContext,
        EVT_HANDLE event) -> DWORD
    {
        auto* reader = static_cast<WindowsEventTracerReader*>(userContext);

        switch (action)
        {
        case EvtSubscribeActionDeliver:
            reader->ProcessEvent(event, reader->GetChannel());
            break;

        case EvtSubscribeActionError:
            //TODO: Handle this error
            break;

        default:
            break;
        }

        return ERROR_SUCCESS;
    };

    EVT_HANDLE subscriptionHandle = EvtSubscribe(
        nullptr,
        nullptr,
        wideStringChannel.c_str(),
        wideStringQuery.c_str(),
        NULL,
        this,
        subscriptionCallback,
        EvtSubscribeToFutureEvents); // EvtSubscribeStartAtOldestRecord

    if (!subscriptionHandle)
    {
        LogError("Failed to subscribe to event log: {}", std::to_string(GetLastError()));
        co_return;
    }

    LogInfo("Subscribed to events for {} channel with query: '{}'", channel, query);

    while (m_keepRunning.load())
    {
        co_await m_logcollector.Wait(std::chrono::milliseconds(m_ChannelsRefreshInterval));
    }

    LogInfo("Unsubscribing to channel '{}'.", channel);
    if (subscriptionHandle)
    {
        EvtClose(subscriptionHandle);
    }
}

void WindowsEventTracerReader::ProcessEvent(EVT_HANDLE event, const std::string channel)
{
    DWORD bufferUsed = 0;
    DWORD propertyCount = 0;

    EvtRender(NULL, event, EvtRenderEventXml, 0, NULL, &bufferUsed, &propertyCount);

    if (bufferUsed > 0)
    {
        std::vector<wchar_t> buffer(bufferUsed);
        if (EvtRender(NULL, event, EvtRenderEventXml, bufferUsed, buffer.data(), &bufferUsed, &propertyCount))
        {
            std::string logString;
            try
            {
                logString = WcharVecToString(buffer);
            }
            catch(const std::exception& e)
            {
                LogError("Cannot convert utf16 string: {}", e.what());
                return;
            }
            m_logcollector.SendMessage(channel, logString, m_collectorType);
        }
    }
}

std::string WindowsEventTracerReader::WcharVecToString(std::vector<wchar_t>& buffer)
{
    buffer.erase(std::remove(buffer.begin(), buffer.end(), L'\0'), buffer.end());
    std::wstring wstr(buffer.begin(), buffer.end());
    std::string result;
    result.reserve(wstr.size() * sizeof(wchar_t));
    std::transform(wstr.begin(), wstr.end(), std::back_inserter(result),
                [](wchar_t wc) -> char {
                    return static_cast<char>(wc);
                });
    return result;
}

}
