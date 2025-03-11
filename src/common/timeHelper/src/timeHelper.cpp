#include "timeHelper.hpp"

#include "stringHelper.hpp"
#include <chrono>
#include <ctime>
#include <iomanip>
#include <pal.h>
#include <sstream>
#include <string>

namespace Utils
{
    constexpr int NINETEEN_HUNDRED_YEAR = 1900;
    constexpr int MILLIS_IN_SECOND = 1000;

    std::string GetTimestamp(const std::time_t& time, const bool utc)
    {
        std::stringstream ss;

        struct tm buf
        {
        };

        // gmtime: result expressed as a UTC time
        tm* localTime {utc ? gmtime_r(&time, &buf) : localtime_r(&time, &buf)};

        if (localTime == nullptr)
        {
            return "1970/01/01 00:00:00";
        }

        // Final timestamp: "YYYY/MM/DD hh:mm:ss"
        // Date
        ss << std::setfill('0') << std::setw(4) << std::to_string(localTime->tm_year + NINETEEN_HUNDRED_YEAR);
        ss << "/";
        ss << std::setfill('0') << std::setw(2) << std::to_string(localTime->tm_mon + 1);
        ss << "/";
        ss << std::setfill('0') << std::setw(2) << std::to_string(localTime->tm_mday);
        // Time
        ss << " ";
        ss << std::setfill('0') << std::setw(2) << std::to_string(localTime->tm_hour);
        ss << ":";
        ss << std::setfill('0') << std::setw(2) << std::to_string(localTime->tm_min);
        ss << ":";
        ss << std::setfill('0') << std::setw(2) << std::to_string(localTime->tm_sec);
        return ss.str();
    }

    std::string getCurrentTimestamp()
    {
        return GetTimestamp(std::time(nullptr));
    }

    std::string GetCompactTimestamp(const std::time_t& time, const bool utc)
    {
        std::stringstream ss;

        struct tm buf
        {
        };

        // gmtime: result expressed as a UTC time
        const tm* localTime {utc ? gmtime_r(&time, &buf) : localtime_r(&time, &buf)};

        if (localTime == nullptr)
        {
            return "1970/01/01 00:00:00";
        }

        // Date
        ss << std::setfill('0') << std::setw(4) << std::to_string(localTime->tm_year + NINETEEN_HUNDRED_YEAR);
        ss << std::setfill('0') << std::setw(2) << std::to_string(localTime->tm_mon + 1);
        ss << std::setfill('0') << std::setw(2) << std::to_string(localTime->tm_mday);
        // Time
        ss << std::setfill('0') << std::setw(2) << std::to_string(localTime->tm_hour);
        ss << std::setfill('0') << std::setw(2) << std::to_string(localTime->tm_min);
        ss << std::setfill('0') << std::setw(2) << std::to_string(localTime->tm_sec);
        return ss.str();
    }

    std::string getCurrentISO8601()
    {
        // Get local time in UTC
        auto now = std::chrono::system_clock::now();
        auto itt = std::chrono::system_clock::to_time_t(now);

        std::ostringstream ss;

        struct tm buf
        {
        };

        tm* localTime = gmtime_r(&itt, &buf);

        if (localTime == nullptr)
        {
            return "1970/01/01 00:00:00";
        }

        ss << std::put_time(localTime, "%FT%T");

        // Get milliseconds from the current time
        auto milliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()).count() % MILLIS_IN_SECOND;

        // ISO 8601
        ss << '.' << std::setfill('0') << std::setw(3) << milliseconds << 'Z';

        return ss.str();
    }

    std::string timestampToISO8601(const std::string& timestamp)
    {
        std::tm tm {};
        std::istringstream ss(timestamp);
        ss >> std::get_time(&tm, "%Y/%m/%d %H:%M:%S");
        if (ss.fail())
        {
            return "";
        }
        const std::time_t time = std::mktime(&tm);

        auto itt = std::chrono::system_clock::from_time_t(time);

        std::ostringstream output;
        struct tm* localTime = gmtime_r(&time, &tm);

        if (localTime == nullptr)
        {
            return "";
        }

        output << std::put_time(localTime, "%FT%T");

        // Get milliseconds from the current time
        auto milliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(itt.time_since_epoch()).count() % MILLIS_IN_SECOND;

        // ISO 8601
        output << '.' << std::setfill('0') << std::setw(3) << milliseconds << 'Z';

        return output.str();
    }

    std::string rawTimestampToISO8601(const std::string& timestamp)
    {
        if (timestamp.empty() || !Utils::isNumber(timestamp))
        {
            return "";
        }

        const std::time_t time = std::stoi(timestamp);
        auto itt = std::chrono::system_clock::from_time_t(time);

        std::ostringstream output;

        struct tm buf
        {
        };

        tm* localTime = gmtime_r(&time, &buf);

        if (localTime == nullptr)
        {
            return "";
        }

        output << std::put_time(localTime, "%FT%T");

        // Get milliseconds from the current time
        auto milliseconds =
            std::chrono::duration_cast<std::chrono::milliseconds>(itt.time_since_epoch()).count() % MILLIS_IN_SECOND;

        // ISO 8601
        output << '.' << std::setfill('0') << std::setw(3) << milliseconds << 'Z';

        return output.str();
    }

    std::chrono::seconds secondsSinceEpoch()
    {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch());
    }

    int64_t getSecondsFromEpoch()
    {
        return std::chrono::duration_cast<std::chrono::seconds>(std::chrono::system_clock::now().time_since_epoch())
            .count();
    };
} // namespace Utils
