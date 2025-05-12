#include "event_reader_win.hpp"

#include <Windows.h>

#include <iomanip>
#include <sstream>

#include <logcollector.hpp>
#include <logger.hpp>

namespace
{
    const std::string COLLECTOR_TYPE = "windows-eventlog";
}

namespace logcollector::winevt
{

    WindowsEventTracerReader::WindowsEventTracerReader(Logcollector& logcollector,
                                                       const std::string channel,
                                                       const std::string query,
                                                       const std::time_t channelRefreshInterval,
                                                       std::shared_ptr<IWinAPIWrapper> winAPI)
        : IReader(logcollector)
        , m_channel(channel)
        , m_query(query.empty() ? "*" : query)
        , m_ChannelsRefreshInterval(channelRefreshInterval)
        , m_winAPI(winAPI ? winAPI : std::make_shared<DefaultWinAPIWrapper>())
    {
    }

    Awaitable WindowsEventTracerReader::Run()
    {
        const auto wideStringChannel = std::wstring(m_channel.begin(), m_channel.end());
        const auto wideStringQuery = std::wstring(m_query.begin(), m_query.end());

        auto subscriptionCallback = [](EVT_SUBSCRIBE_NOTIFY_ACTION action, PVOID userContext, EVT_HANDLE event) -> DWORD
        {
            auto* reader = static_cast<WindowsEventTracerReader*>(userContext);

            switch (action)
            {
                case EvtSubscribeActionDeliver: reader->ProcessEvent(event); break;

                case EvtSubscribeActionError:
                    // Should this error be handled
                    break;

                default: break;
            }

            return ERROR_SUCCESS;
        };

        EVT_HANDLE subscriptionHandle = m_winAPI->EvtSubscribe(nullptr,
                                                               nullptr,
                                                               wideStringChannel.c_str(),
                                                               wideStringQuery.c_str(),
                                                               nullptr,
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

        co_return;
    }

    void WindowsEventTracerReader::Stop()
    {
        m_keepRunning.store(false);
    }

    void WindowsEventTracerReader::ProcessEvent(EVT_HANDLE event)
    {
        DWORD bufferUsed = 0;
        DWORD propertyCount = 0;

        m_winAPI->EvtRender(nullptr, event, EvtRenderEventXml, 0, nullptr, &bufferUsed, &propertyCount);

        if (bufferUsed > 0)
        {
            std::vector<wchar_t> buffer(bufferUsed);
            if (m_winAPI->EvtRender(
                    nullptr, event, EvtRenderEventXml, bufferUsed, buffer.data(), &bufferUsed, &propertyCount))
            {
                std::string logString;
                try
                {
                    logString = WcharVecToString(buffer);
                }
                catch (const std::exception& e)
                {
                    LogError("Cannot convert utf16 string: {}", e.what());
                    return;
                }

                try
                {
                    m_logcollector.PushMessage(m_channel, logString, COLLECTOR_TYPE);
                }
                catch (const std::exception& e)
                {
                    LogError("Cannot push message '{}' into the queue: {}", logString, e.what());
                }
            }
        }
    }

    std::string WindowsEventTracerReader::WcharVecToString(std::vector<wchar_t>& buffer)
    {
        if (buffer.empty() || buffer.back() != L'\0')
        {
            buffer.push_back(L'\0');
        }
        std::wstring wstr(buffer.data());

        int utf8Size = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
        if (utf8Size == 0)
        {
            LogError("Error in WideCharToMultiByte conversion");
            return "";
        }

        std::vector<char> utf8Buffer(utf8Size);
        WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, utf8Buffer.data(), utf8Size, nullptr, nullptr);

        return std::string(utf8Buffer.data(), utf8Size - 1);
    }

} // namespace logcollector::winevt
