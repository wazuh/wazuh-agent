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
    // TODO: add lambda for loop break
    while (true)
    {
        for (size_t i = 0; i < m_channelsList.size(); i++)
        {
            QueryEvents(m_channelsList.at(i), m_queriesList.at(i));
        }

        co_await m_logcollector.Wait(std::chrono::milliseconds(m_ChannelsRefreshInterval));
    }
}

Awaitable WindowsEventTracerReader::QueryEvents(const std::string channel, const std::string query)
{
    // Load bookmark if exists
    EVT_HANDLE bookmarkHandle = LoadBookmark();

    //TODO: rework this casting
    std::wstring wide_string_channel = std::wstring(channel.begin(), channel.end());
    std::wstring wide_string_query = std::wstring(query.begin(), query.end());
    EVT_HANDLE eventQueryHandle = EvtQuery( NULL, wide_string_channel.c_str(), wide_string_query.c_str(),
        EvtQueryChannelPath | EvtQueryReverseDirection);

    if (eventQueryHandle == NULL)
    {
        // TODO: Logging fix
        // LogError("Failed to query event log: {}", GetLastError());
        std::cerr << "Failed to query event log: " << GetLastError() << std::endl;
        co_return;
    }

    EVT_HANDLE events[10];
    DWORD eventCount = 0;

    while (EvtNext(eventQueryHandle, 10, events, INFINITE, 0, &eventCount))
    {
        for (DWORD i = 0; i < eventCount; ++i)
        {
            ProcessEvent(events[i], channel);

            if(m_bookmarkEnabled)
            {
                // Save the last event as a bookmark
                bookmarkHandle = CreateBookmark(events[i], bookmarkHandle);
                SaveBookmark(bookmarkHandle);
            }
            EvtClose(events[i]);
        }
        //TODO: check using logcollec
        co_await m_logcollector.Wait(std::chrono::milliseconds(m_eventsProcessingInterval));
    }

    EvtClose(eventQueryHandle);
    if (bookmarkHandle)
    {
        EvtClose(bookmarkHandle);
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
            // TODO: Logging fix
            // LogTrace("Event: {}",buffer.data());
            std::wcout << L"Event: " << buffer.data() << std::endl;
            const std::string log {};
            // m_logcollector.SendMessage(channel, log, m_collectorType);
        }
    }
}

EVT_HANDLE WindowsEventTracerReader::CreateBookmark(EVT_HANDLE event, EVT_HANDLE existingBookmark)
{
    EVT_HANDLE bookmark = existingBookmark ? existingBookmark : EvtCreateBookmark(NULL);
    if (!EvtUpdateBookmark(bookmark, event))
    {
        // TODO: Logging fix
        // LogError("Failed to update bookmark: {}", GetLastError());
        std::cerr << "Failed to update bookmark: " << GetLastError() << std::endl;
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
        std::wofstream file(bookmarkFile_);
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
        std::wifstream file(bookmarkFile_);
        if (file.is_open())
        {
            std::wstringstream buffer;
            buffer << file.rdbuf();
            std::wstring bookmarkXML = buffer.str();
            file.close();
            return EvtCreateBookmark(bookmarkXML.c_str());
        }
    }

    return NULL;
}

