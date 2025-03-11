#pragma once

#include <chrono>
#include <ctime>
#include <string>

namespace Utils
{
    /// @brief Get timestamp
    /// @param time Time to convert
    /// @param utc If true, the time will be expressed as a UTC time
    /// @return Timestamp
    std::string GetTimestamp(const std::time_t& time, const bool utc = true);

    /// @brief Get current timestamp
    /// @return Current timestamp
    std::string getCurrentTimestamp();

    /// @brief Get a compact timestamp
    /// @param time Time to convert
    /// @param utc If true, the time will be expressed as a UTC time
    /// @return Compact timestamp. Format: "YYYYMMDDhhmmss"
    std::string GetCompactTimestamp(const std::time_t& time, const bool utc = true);

    /// @brief Get current timestamp in ISO 8601 format
    /// @return Current timestamp in ISO 8601 format
    std::string getCurrentISO8601();

    /// @brief Convert timestamp to ISO 8601 format
    /// @param timestamp Timestamp to convert
    /// @return Timestamp in ISO 8601 format
    std::string timestampToISO8601(const std::string& timestamp);

    /// @brief Convert raw timestamp to ISO 8601 format
    /// @param timestamp Raw timestamp to convert
    /// @return Raw timestamp in ISO 8601 format
    std::string rawTimestampToISO8601(const std::string& timestamp);

    /// @brief Get seconds from epoch, since 1970-01-01 00:00:00 UTC
    /// @return seconds from epoch as std::chrono::seconds
    std::chrono::seconds secondsSinceEpoch();

    /// @brief Get seconds from epoch, since 1970-01-01 00:00:00 UTC
    /// @return seconds from epoch as int64_t
    int64_t getSecondsFromEpoch();
} // namespace Utils
