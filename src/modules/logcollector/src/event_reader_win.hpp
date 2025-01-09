#pragma once

#include <ctime>
#include <codecvt>
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
                            const std::string channel,
                            const std::string query,
                            const std::time_t channelRefreshInterval);

    /// @brief Runs the file reader
    /// @return Awaitable result
    Awaitable Run() override;

    //TODO: doc
    void Stop() override;

    // Main function to execute the event query with bookmarks and filters
    Awaitable QueryEvents(const std::string channel, const std::string query);

    //TODO: doc
    std::string GetChannel() { return m_channel; };

private:
    // Process an individual event and print its XML representation
    void ProcessEvent(EVT_HANDLE event, const std::string channel);

    //TODO: doc
    std::string WcharVecToString(std::vector<wchar_t>& buffer);

    /// @brief
    std::string m_channel;

    /// @brief
    std::string m_query;

    /// @brief
    std::time_t m_ChannelsRefreshInterval;

    /// @brief collector type
    const std::string m_collectorType = "eventchannel";
};
} // namespace logcollector
