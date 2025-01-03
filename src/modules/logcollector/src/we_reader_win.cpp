#include "we_reader_win.hpp"

#include <logger.hpp>

using namespace logcollector;

WindowsEventTracerReader::WindowsEventTracerReader(Logcollector &logcollector,
                           const std::vector<std::string> channels,
                           const std::vector<std::string> queries,
                           const std::time_t channelRefreshInterval,
                           bool bookmarkEnabled) :
    IReader(logcollector),
    m_channelsList(channels),
    m_queriesList(queries),
    m_ChannelsRefreshInterval(channelRefreshInterval),
    m_bookmarkEnabled(bookmarkEnabled) { }

Awaitable WindowsEventTracerReader::Run()
{
    //TODO: find a clearer way of connecting this with outside modules
    bool keepRunning = true;
    std::function<bool()> shouldContinue = [&keepRunning]() { return keepRunning; };

    for (size_t i = 0; i < m_channelsList.size(); i++)
    {
        m_logcollector.EnqueueTask(QueryEvents(m_channelsList.at(i), m_queriesList.at(i), shouldContinue));
    }
    co_return;
}

Awaitable WindowsEventTracerReader::QueryEvents(const std::string channel, const std::string query, std::function<bool()> shouldContinue)
{
    EVT_HANDLE bookmarkHandle = LoadBookmark();

    //TODO: rework this casting
    std::wstring wideStringChannel = std::wstring(channel.begin(), channel.end());
    std::wstring wideStringQuery = std::wstring(query.begin(), query.end());

    EVT_HANDLE eventQueryHandle = EvtQuery( NULL, wideStringChannel.c_str(), wideStringQuery.c_str(),
        EvtQueryChannelPath | EvtQueryForwardDirection);

    if (eventQueryHandle == NULL)
    {
        LogError("Failed to query event log: {}", std::to_string(GetLastError()));
        co_return;
    }

    LogInfo("Querying events for {} channel with query: '{}'", channel, query);

    EVT_HANDLE events[10];
    DWORD eventCount = 0;

    while (shouldContinue())
    {
        if (EvtNext(eventQueryHandle, 10, events, 1000, 0, &eventCount))
        {
            for (DWORD i = 0; i < eventCount; ++i)
            {
                ProcessEvent(events[i], channel);
                if(m_bookmarkEnabled)
                {
                    bookmarkHandle = CreateBookmark(events[i], bookmarkHandle);
                    SaveBookmark(bookmarkHandle);
                }
                EvtClose(events[i]);
            }
        }
        else
        {
            DWORD error = GetLastError();
            if (error == ERROR_NO_MORE_ITEMS)
            {
                //TODO: delete or make it trace
                LogInfo("No more events. Waiting for new events...");
                co_await m_logcollector.Wait(std::chrono::milliseconds(m_ChannelsRefreshInterval));
            }
            else
            {
                LogError("EvtNext failed with error: {}" , std::to_string(error));
                break;
            }
        }
    }

    if (bookmarkHandle)
    {
        EvtClose(bookmarkHandle);
    }

    if (eventQueryHandle)
    {
        EvtClose(eventQueryHandle);
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
            //TODO: delete later
            LogInfo("Creating bookark");
            return EvtCreateBookmark(bookmarkXML.c_str());
        }
        else
        {
            LogError("Couldn't open bookmark file: {}", m_bookmarkFile);
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
