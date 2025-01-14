#pragma once

#include <boost/asio/awaitable.hpp>
#include <logcollector.hpp>

namespace logcollector {

using Awaitable = boost::asio::awaitable<void>;

/// @brief Interface for log readers
class IReader {
public:
    /// @brief Constructor
    /// @param logcollector Logcollector instance
    IReader(Logcollector& logcollector) :
        m_logcollector(logcollector) { }

    /// @brief Destructor
    virtual ~IReader() = default;

    /// @brief Runs the log reader
    /// @return Awaitable result
    virtual Awaitable Run() = 0;

    /// @brief Stops the log reader
    virtual void Stop() = 0;

protected:
    /// @brief Indicates if the log reader should keep running
    std::atomic<bool> m_keepRunning = true;

    /// @brief Logcollector instance
    Logcollector& m_logcollector;
};

}
