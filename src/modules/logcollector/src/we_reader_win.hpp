#ifdef _WIN32
#pragma once

#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <WinSock2.h>
#include <Windows.h>
#include <winevt.h>

#include "reader.hpp"

#pragma comment(lib, "wevtapi.lib")

namespace logcollector {

/// @brief Windows Event Tracer Reader class
/// TODO: doc
class WindowsEventTracerReader : public IReader
{
public:
    /// @brief Constructor for the Windows Event Tracer Reader
    /// @param logcollector Log collector instance
    /// @param channels List of channel names
    /// @param queries List of queries
    /// @param reloadInterval
    /// @param bookmarkEnabled
    WindowsEventTracerReader(Logcollector &logcollector,
                            const std::vector<std::string> channels,
                            const std::vector<std::string> queries,
                            const std::time_t reloadInterval,
                            bool bookmarkEnabled);

    /// @brief Runs the file reader
    /// @return Awaitable result
    Awaitable Run() override;

    // Main function to execute the event query with bookmarks and filters
    Awaitable QueryEvents(const std::string channel, const std::string query);

private:
    // Process an individual event and print its XML representation
    void ProcessEvent(EVT_HANDLE event, const std::string channel);

    // Create a bookmark for the current event
    EVT_HANDLE CreateBookmark(EVT_HANDLE event, EVT_HANDLE existingBookmark = NULL);

    // Save bookmark to file
    void SaveBookmark(EVT_HANDLE bookmarkHandle);

    // Load bookmark from file (if it exists)
    EVT_HANDLE LoadBookmark();

    std::vector<std::string> m_channelsList;

    std::vector<std::string> m_queriesList;

    std::wstring bookmarkFile_;

    std::string m_channel;

    std::string m_query;

    /// @brief
    std::time_t m_ChannelsRefreshInterval;

    /// @brief
    std::time_t m_eventsProcessingInterval;

    bool m_bookmarkEnabled;

    int m_availableEvents = 0;

    const std::string m_collectorType = "eventchannel";
};
} // namespace logcollector
#endif
