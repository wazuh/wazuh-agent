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

    const auto bookmarkEnabled = configurationParser->GetConfig<bool>("logcollector", "use_bookmark").value_or(config::logcollector::DEFAULT_USE_BOOKMARK);

    const auto windowsConfig = configurationParser->GetConfig<std::vector<std::map<std::string, std::string>>>("logcollector", "windows").value_or(
        std::vector<std::map<std::string, std::string>> {});

    for (auto& entry : windowsConfig)
    {
        auto channel = entry.at("channel");
        auto query = entry.at("query");
        logcollector.AddReader(std::make_shared<WindowsEventTracerReader>(logcollector, channel, query, refreshInterval, bookmarkEnabled));
    }
}

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
            // Creates a single file per instance
            std::string filePath = config::DEFAULT_DATA_PATH;
            std::string sanitizedChannel = channel;
            std::replace(sanitizedChannel.begin(), sanitizedChannel.end(), '\\', '_');
            m_bookmarkFile = filePath + "\\" + sanitizedChannel + Base64Encode(query) + ".bmk";;
        }
    }

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
        if (bookmarkHandle)
        {
            EvtClose(bookmarkHandle);
        }
        co_return;
    }

    LogInfo("Subscribed to events for {} channel with query: '{}'", channel, query);

    while (m_keepRunning.load())
    {
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
                LogTrace("Empty bookmark file");
            }
            else
            {
                LogInfo("Creating bookark");
                return EvtCreateBookmark(bookmarkXML.c_str());
            }
        }
        else
        {
            LogWarn("Couldn't open bookmark file '{}'.",m_bookmarkFile);
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

//TODO: replace by boost library implementation
std::string WindowsEventTracerReader::Base64Encode(const std::string& input)
{
    static const char* chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
    std::string result;
    unsigned int val = 0;
    int valb = -6;
    for (unsigned char c : input)
    {
        val = (val << 8) + c;
        valb += 8;
        while (valb >= 0)
        {
            result.push_back(chars[(val >> valb) & 0x3F]);
            valb -= 6;
        }
    }

    if (valb > -6)
    {
        result.push_back(chars[((val << 8) >> (valb + 8)) & 0x3F]);
    }

    while (result.size() % 4)
    {
        result.push_back('=');
    }

    return result;
}

}

