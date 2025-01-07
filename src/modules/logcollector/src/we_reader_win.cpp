#include "we_reader_win.hpp"

#include <logger.hpp>

using namespace logcollector;

WindowsEventTracerReader::WindowsEventTracerReader(Logcollector &logcollector,
                           const std::string channel,
                           const std::string query,
                           const std::time_t channelRefreshInterval,
                           bool bookmarkEnabled) :
    IReader(logcollector),
    m_channel(channel),
    m_query(query),
    m_ChannelsRefreshInterval(channelRefreshInterval),
    m_bookmarkEnabled(bookmarkEnabled)
    {
        if(m_bookmarkEnabled)
        {
            std::string fileName {"\\bookmark.xml"};
            std::string filePath = config::DEFAULT_DATA_PATH;
            m_bookmarkFile = filePath + fileName;
        }
    }

Awaitable WindowsEventTracerReader::Run()
{
    m_logcollector.EnqueueTask(QueryEvents(m_channel, m_query));
    co_return;
}

void WindowsEventTracerReader::Stop()
{
    //TODO
    m_keepRunning = false;
}

Awaitable WindowsEventTracerReader::QueryEvents(const std::string channel, const std::string query)
{
    EVT_HANDLE bookmarkHandle = LoadBookmark();

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
            if (reader->IsBookmarkEnabled())
            {
                EVT_HANDLE bookmarkHandle = reader->CreateBookmark(event, nullptr);
                reader->SaveBookmark(bookmarkHandle);
                EvtClose(bookmarkHandle);
            }
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
        wideStringQuery.empty() ? nullptr : wideStringQuery.c_str(),
        bookmarkHandle,
        this,
        subscriptionCallback,
        m_bookmarkEnabled && bookmarkHandle? EvtSubscribeStartAfterBookmark : EvtSubscribeStartAtOldestRecord);

    if (!subscriptionHandle)
    {
        LogError("Failed to subscribe to event log: {}", std::to_string(GetLastError()));
        co_return;
    }

    LogInfo("Subscribed to events for {} channel with query: '{}'", channel, query);

    while (true)
    {
        if (!m_keepRunning)
        {
            LogInfo("Stopping subscription based on external signal...");
            break;
        }

        co_await m_logcollector.Wait(std::chrono::milliseconds(m_ChannelsRefreshInterval));
    }

    LogInfo("Unsubscribing to channel '{}'.", channel);
    if (bookmarkHandle)
    {
        EvtClose(bookmarkHandle);
    }
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

EVT_HANDLE WindowsEventTracerReader::CreateBookmark(EVT_HANDLE event, EVT_HANDLE existingBookmark)
{
    EVT_HANDLE bookmark = existingBookmark ? existingBookmark : EvtCreateBookmark(NULL);
    if (!EvtUpdateBookmark(bookmark, event))
    {
        LogError("Failed to update bookmark: {}", std::to_string(GetLastError()));
    }
    return bookmark;
}

void WindowsEventTracerReader::SaveBookmark(EVT_HANDLE bookmarkHandle)
{
    DWORD bufferUsed = 0;

    EvtRender(NULL, bookmarkHandle, EvtRenderBookmark, 0, NULL, &bufferUsed, NULL);
    std::vector<wchar_t> buffer(bufferUsed);
    if (EvtRender(NULL, bookmarkHandle, EvtRenderBookmark, bufferUsed, buffer.data(), &bufferUsed, NULL))
    {
        std::wofstream file(m_bookmarkFile);
        if (file.is_open())
        {
            file.write(buffer.data(), bufferUsed / sizeof(wchar_t));
            file.close();
        }
    }
}

EVT_HANDLE WindowsEventTracerReader::LoadBookmark()
{
    if(m_bookmarkEnabled)
    {
        std::wifstream file(m_bookmarkFile);
        if (file.is_open())
        {
            std::wstringstream buffer;
            buffer << file.rdbuf();
            std::wstring bookmarkXML = buffer.str();
            file.close();
            if(bookmarkXML.empty())
            {
                LogTrace("Empty bookark file, exiting");
            }
            else
            {
                LogInfo("Creating bookark");
                return EvtCreateBookmark(bookmarkXML.c_str());
            }
        }
        else
        {
            LogWarn("Couldn't open bookmark file.");
        }
    }

    return NULL;
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
