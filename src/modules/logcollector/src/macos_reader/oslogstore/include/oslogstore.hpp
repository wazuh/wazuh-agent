#pragma once

#include <memory>
#include <string>
#include <vector>

/// @class IOSLogStoreWrapper
/// @brief A C++ abstract class for a wrapper to retrieve LogEntries
class IOSLogStoreWrapper
{
public:
    /// @struct LogEntry
    /// @brief Represents a single log entry with timestamp and message details.
    ///
    /// This structure encapsulates the essential components of a log entry, including the timestamp in both
    /// seconds since the Unix epoch and a formatted date string, as well as the log message itself.
    struct LogEntry
    {
        /// @brief The date and time of the log entry in seconds since the Unix epoch.
        double dateInSeconds;

        /// @brief The formatted date and time string of the log entry.
        std::string date;

        /// @brief The log message content.
        std::string log;
    };

    /// @enum LogLevel
    /// @brief Enumerates the possible log levels for filtering log entries.
    enum LogLevel
    {
        Undefined,
        Debug,
        Info,
        Notice,
        Error,
        Fault,
    };

    /// @brief Destructor, virtual, default.
    virtual ~IOSLogStoreWrapper() = default;

    /// @param startTimeSeconds The start time in seconds since the Unix epoch (e.g., 1672531200.0 for Jan 1, 2023).
    /// @param query An optional query string using NSPredicate syntax for additional filtering.
    /// @param logLevel The desired log level to filter entries.
    ///
    /// @return A `std::vector` of `LogEntry` structs containing the date and log message for each matching entry.
    ///
    /// @throws std::runtime_error If the OSLogStore cannot be accessed or log enumeration fails.
    virtual std::vector<LogEntry> AllEntries(const double startTimeSeconds,
                                             const std::string& query,
                                             const LogLevel logLevel = LogLevel::Undefined) = 0;
};

/// @brief Forward declaration of the OSLogStoreWrapperImpl class.
class OSLogStoreWrapperImpl;

/// @class OSLogStoreWrapper
/// @brief A C++ wrapper for accessing and enumerating OS logs using OSLogStore APIs.
///
/// This class provides an interface to retrieve and manage system log entries based on specified criteria such as
/// time range, query filters, and log levels. It encapsulates the underlying Objective-C OSLogStore functionalities,
/// facilitating seamless integration within C++ applications.
class OSLogStoreWrapper : public IOSLogStoreWrapper
{
public:
    /// @brief Constructor
    OSLogStoreWrapper();

    /// @brief  Destructor
    ~OSLogStoreWrapper();

    /// @copydoc IOSLogStoreWrapper::AllEntries
    std::vector<LogEntry> AllEntries(const double startTimeSeconds,
                                     const std::string& query,
                                     const LogLevel logLevel = LogLevel::Undefined) override;

private:
    std::unique_ptr<OSLogStoreWrapperImpl> m_impl;
};
