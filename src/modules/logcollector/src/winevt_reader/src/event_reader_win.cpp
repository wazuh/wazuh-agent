#include "event_reader_win.hpp"

#include <sstream>
#include <iomanip>

#include <logger.hpp>
#include "../../../include/logcollector.hpp"

namespace logcollector::winevt
{

WindowsEventTracerReader::WindowsEventTracerReader(Logcollector &logcollector,
                           const std::string channel,
                           const std::string query,
                           const std::time_t channelRefreshInterval,
                           std::shared_ptr<IWinAPIWrapper> winAPI) :
    IReader(logcollector),
    m_channel(channel),
    m_query(query),
    m_ChannelsRefreshInterval(channelRefreshInterval),
    m_winAPI(winAPI ? winAPI : std::make_shared<DefaultWinAPIWrapper>()){ }

Awaitable WindowsEventTracerReader::Run()
{
    m_logcollector.EnqueueTask(QueryEvents());
    co_return;
}

void WindowsEventTracerReader::Stop()
{
    m_keepRunning.store(false);
}

Awaitable WindowsEventTracerReader::QueryEvents()
{
    const auto wideStringChannel = std::wstring(m_channel.begin(), m_channel.end());
    const auto wideStringQuery = std::wstring(m_query.begin(), m_query.end());

    auto subscriptionCallback = [](EVT_SUBSCRIBE_NOTIFY_ACTION action, PVOID userContext,
        EVT_HANDLE event) -> DWORD
    {
        auto* reader = static_cast<WindowsEventTracerReader*>(userContext);

        switch (action)
        {
        case EvtSubscribeActionDeliver:
            reader->ProcessEvent(event);
            break;

        case EvtSubscribeActionError:
            //Should this error be handled
            break;

        default:
            break;
        }

        return ERROR_SUCCESS;
    };

    EVT_HANDLE subscriptionHandle = m_winAPI->EvtSubscribe(
        nullptr,
        nullptr,
        wideStringChannel.c_str(),
        wideStringQuery.c_str(),
        NULL,
        this,
        subscriptionCallback,
        EvtSubscribeToFutureEvents);

    if (!subscriptionHandle)
    {
        LogError("Failed to subscribe to event log: {}", std::to_string(GetLastError()));
        co_return;
    }

    LogInfo("Subscribed to events for '{}' channel with query: '{}'", m_channel, m_query);

    while (m_keepRunning.load())
    {
        co_await m_logcollector.Wait(std::chrono::milliseconds(m_ChannelsRefreshInterval));
    }

    LogInfo("Unsubscribing to channel '{}'.", m_channel);
    if (subscriptionHandle)
    {
        m_winAPI->EvtClose(subscriptionHandle);
    }
}

void WindowsEventTracerReader::ProcessEvent(EVT_HANDLE event)
{
    DWORD bufferUsed = 0;
    DWORD propertyCount = 0;

    m_winAPI->EvtRender(NULL, event, EvtRenderEventXml, 0, NULL, &bufferUsed, &propertyCount);

    if (bufferUsed > 0)
    {
        std::vector<wchar_t> buffer(bufferUsed);
        if (m_winAPI->EvtRender(NULL, event, EvtRenderEventXml, bufferUsed, buffer.data(), &bufferUsed, &propertyCount))
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
            m_logcollector.SendMessage(m_channel, logString, m_collectorType);
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
