#pragma once

#include <boost/asio/awaitable.hpp>

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

protected:
    /// @brief Logcollector instance
    Logcollector& m_logcollector;
};

}
