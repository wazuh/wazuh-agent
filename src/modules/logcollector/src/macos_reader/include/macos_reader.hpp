#pragma once

#include "../../reader.hpp"

#include <config.h>
#include <logcollector.hpp>
#include <oslogstore.hpp>

#include <boost/asio/awaitable.hpp>

#include <atomic>
#include <chrono>
#include <ctime>
#include <memory>
#include <string>
#include <vector>

namespace logcollector
{

/// @class MacOSReader
/// @brief A reader class for collecting and processing OS logs.
///
/// The `MacOSReader` class implements the `IReader` interface to provide functionality for
/// reading and processing system log entries using a `OSLogStoreWrapper`. It supports
/// filtering logs based on log levels, custom queries, and specific log types. The reader
/// operates asynchronously, allowing it to be integrated seamlessly into event-driven
/// applications.
class MacOSReader : public IReader
{
public:
    /// @brief Constructs a new `MacOSReader` instance.
    /// @param logcollector Reference to the `Logcollector` instance managing the log reading process.
    /// @param waitInMillis Duration in milliseconds to wait before processing log files.
    /// @param logLevel The minimum log level to filter log entries.
    /// @param query An optional query string using `NSPredicate` syntax for additional log filtering.
    /// @param logTypes A vector of log type strings to further filter log entries.
    MacOSReader(
        Logcollector& logcollector,
        const std::time_t waitInMillis = config::logcollector::DEFAULT_FILE_WAIT,
        const std::string& logLevel = "",
        const std::string& query = "",
        const std::vector<std::string>& logTypes = {}
    );

    /// @brief Constructs a new `MacOSReader` instance, allowing dependency injection for the `IOSLogStoreWrapper`.
    /// @param osLogStoreWrapper A unique pointer to an instance of `IOSLogStoreWrapper`, used for interacting with the OS log store.
    /// @param logcollector Reference to the `Logcollector` instance managing the log reading process.
    /// @param waitInMillis Duration in milliseconds to wait before processing log files.
    /// @param logLevel The minimum log level to filter log entries.
    /// @param query An optional query string using `NSPredicate` syntax for additional log filtering.
    /// @param logTypes A vector of log type strings to further filter log entries.
    MacOSReader(
        std::unique_ptr<IOSLogStoreWrapper> osLogStoreWrapper,
        Logcollector& logcollector,
        const std::time_t waitInMillis = config::logcollector::DEFAULT_FILE_WAIT,
        const std::string& logLevel = "",
        const std::string& query = "",
        const std::vector<std::string>& logTypes = {}
    );

    /// @brief Destructor for `MacOSReader`.
    ~MacOSReader() override = default;

    /// @brief Runs the log reader asynchronously.
    /// @return An `Awaitable` object representing the asynchronous operation.
    Awaitable Run() override;

    /// @brief Stops the log reader.
    void Stop() override;

private:
    /// @brief Sets the log level for filtering log entries.
    ///
    /// Updates the internal log level based on the provided string. This method translates the
    /// string representation of the log level to the corresponding `OSLogStoreWrapper::LogLevel`
    /// enumeration value.
    ///
    /// @param logLevel A string representing the desired log level (e.g., "Info", "Error").
    void SetLogLevel(const std::string& logLevel);

    /// @brief Sets the query and log types for filtering log entries.
    ///
    /// Configures the internal query string and log types used to filter log entries. The query
    /// string utilizes `NSPredicate` syntax to define complex filtering criteria, while the log
    /// types specify the categories of logs to include.
    ///
    /// @param query An `NSPredicate`-formatted string for additional log filtering.
    /// @param logTypes A vector of strings specifying the types of logs to include.
    void SetQuery(const std::string& query, const std::vector<std::string>& logTypes);

    std::unique_ptr<IOSLogStoreWrapper> m_osLogStoreWrapper;
    OSLogStoreWrapper::LogLevel m_logLevel;
    std::string m_query;
    double m_lastLogEntryTimeInSecondsSince1970;
    std::time_t m_waitInMillis;
};

} // namespace logcollector
