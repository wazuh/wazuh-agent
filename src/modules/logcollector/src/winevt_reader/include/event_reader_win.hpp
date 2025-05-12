#pragma once

#include <codecvt>
#include <ctime>
#include <fstream>
#include <iostream>
#include <memory>
#include <sstream>
#include <string>
#include <vector>

#include <reader.hpp>
#include <winevt_wrapper_win.hpp>

#pragma comment(lib, "wevtapi.lib")

namespace logcollector::winevt
{

    /// @brief Windows Event Tracer Reader class
    class WindowsEventTracerReader : public IReader
    {
    public:
        /// @brief Constructor for the Windows Event Tracer Reader
        /// @param logcollector Log collector instance.
        /// @param channel Channel name.
        /// @param query Query.
        /// @param channelRefreshInterval channel query refresh interval in millisecconds.
        /// @param winAPI wrapper of winevt methods.
        WindowsEventTracerReader(Logcollector& logcollector,
                                 const std::string channel,
                                 const std::string query,
                                 const std::time_t channelRefreshInterval,
                                 std::shared_ptr<IWinAPIWrapper> winAPI = nullptr);

        /// @copydoc IReader::Run
        Awaitable Run() override;

        /// @copydoc IReader::Stop
        void Stop() override;

    private:
        ///@brief Process an individual event and print its XML representation.
        /// @param event subscription handle event.
        void ProcessEvent(EVT_HANDLE event);

        /// @brief Function for wchar to string convertion.
        /// @param buffer wchar vector as input.
        /// @return buffer converted to std::string.
        std::string WcharVecToString(std::vector<wchar_t>& buffer);

        /// @brief channel name.
        std::string m_channel;

        /// @brief query string.
        std::string m_query;

        /// @brief channel query refresh interval in millisecconds.
        std::time_t m_ChannelsRefreshInterval;

        /// @brief winevt wrapper for overriding while testing.
        std::shared_ptr<IWinAPIWrapper> m_winAPI;
    };

} // namespace logcollector::winevt
