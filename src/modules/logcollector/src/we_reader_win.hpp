#ifdef _WIN32
#pragma once

#include <ctime>
#include <list>
#include <string>

#include "reader.hpp"

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
                            const std::list<std::string> channels,
                            const std::list<std::string> queries,
                            const std::time_t reloadInterval,
                            bool bookmarkEnabled);

    /// @brief Runs the file reader
    /// @return Awaitable result
    Awaitable Run() override;

    // TODO: doc
    Awaitable ReadEventChannel();

private:
    /// @brief
    std::time_t m_ReaderReloadInterval;

    std::list<std::string> m_channelsList;
    
    std::list<std::string> m_queriesList;

    bool m_bookmarkEnabled;

    int m_availableEvents = 0;

    const std::string m_collectorType = "eventchannel";
};

} // namespace logcollector
#endif
