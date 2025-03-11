#include "windowsHelper.hpp"

#include "stringHelper.hpp"
#include "timeHelper.hpp"
#include <memory>
#include <pal.h>
#include <time.h>
#include <versionhelpers.h>

namespace Utils
{
    bool isVistaOrLater()
    {
        static const bool ret {IsWindowsVistaOrGreater()};
        return ret;
    }

    // https://en.wikipedia.org/wiki/ISO_8601#Calendar_dates
    // https://en.wikipedia.org/wiki/ISO_8601#Combined_date_and_time_representations
    std::string normalizeTimestamp(const std::string& dateISO8601CalendarDateFormat,
                                   const std::string& dateISO8601CombinedFormat)
    {
        std::string normalizedTimestamp;

        if (dateISO8601CalendarDateFormat.size() != INSTALLDATE_REGISTRY_VALUE_SIZE)
        {
            throw std::runtime_error("Invalid dateISO8601CalendarDateFormat size.");
        }

        if (!isNumber(dateISO8601CalendarDateFormat))
        {
            throw std::runtime_error("Invalid dateISO8601CalendarDateFormat format.");
        }

        const auto pos = dateISO8601CombinedFormat.find(' ');

        if (pos != std::string::npos)
        {
            // Substracts "YYYY/MM/DD" from "YYYY/MM/DD hh:mm:ss" string.
            auto dateTrimmed = dateISO8601CombinedFormat.substr(0, pos);
            // Substracts "hh:mm:ss" from "YYYY/MM/DD hh:mm:ss" string.
            auto timeTrimmed = dateISO8601CombinedFormat.substr(pos + 1);

            // Converts "YYYY/MM/DD" string to "YYYYMMDD".
            Utils::replaceAll(dateTrimmed, "/", "");
            // Converts "hh:mm:ss" string to "hhmmss".
            Utils::replaceAll(timeTrimmed, ":", "");

            if (dateTrimmed.size() == INSTALLDATE_REGISTRY_VALUE_SIZE || timeTrimmed.size() == HOURMINSEC_VALUE_SIZE)
            {
                if (dateTrimmed.compare(dateISO8601CalendarDateFormat) == 0)
                {
                    normalizedTimestamp = dateISO8601CombinedFormat;
                }
                else
                {
                    tm local_time_s {};

                    // Parsing YYYYMMDD date format string.
                    local_time_s.tm_year = std::stoi(dateISO8601CalendarDateFormat.substr(0, 4)) - REFERENCE_YEAR;
                    local_time_s.tm_mon = std::stoi(dateISO8601CalendarDateFormat.substr(4, 2)) - 1;
                    local_time_s.tm_mday = std::stoi(dateISO8601CalendarDateFormat.substr(6, 2));
                    local_time_s.tm_hour = 0;
                    local_time_s.tm_min = 0;
                    local_time_s.tm_sec = 0;
                    time_t local_time = mktime(&local_time_s);

                    normalizedTimestamp = Utils::GetTimestamp(local_time, false);
                }
            }
            else
            {
                throw std::runtime_error("Invalid dateISO8601CombinedFormat format.");
            }
        }
        else
        {
            throw std::runtime_error("Invalid dateISO8601CombinedFormat date/time separator.");
        }

        return normalizedTimestamp;
    }

    // Reference: https://www.dmtf.org/sites/default/files/standards/documents/DSP0134_2.6.0.pdf
    std::string getSerialNumberFromSmbios(const BYTE* rawData, const DWORD rawDataSize)
    {
        std::string serialNumber;
        DWORD offset {0};

        if (nullptr != rawData)
        {
            std::unique_ptr<BYTE[]> tmpBuffer {std::make_unique<BYTE[]>(rawDataSize + 1)};
            memcpy(tmpBuffer.get(), rawData, rawDataSize);

            while (offset < rawDataSize && serialNumber.empty())
            {
                if (offset + sizeof(SMBIOSStructureHeader) >= rawDataSize)
                {
                    break;
                }

                SMBIOSStructureHeader header {};
                memcpy(&header, tmpBuffer.get() + offset, sizeof(SMBIOSStructureHeader));

                if (offset + header.FormattedAreaLength >= rawDataSize ||
                    offset + sizeof(SMBIOSBaseboardInfoStructure) >= rawDataSize)
                {
                    break;
                }

                if (BASEBOARD_INFORMATION_TYPE == header.Type)
                {
                    SMBIOSBaseboardInfoStructure info {};
                    memcpy(&info, tmpBuffer.get() + offset, sizeof(SMBIOSBaseboardInfoStructure));
                    offset += info.FormattedAreaLength;

                    for (BYTE i = 1; i < info.SerialNumber; ++i)
                    {
                        const char* tmp {reinterpret_cast<const char*>(tmpBuffer.get() + offset)};

                        if (offset < rawDataSize)
                        {
                            const auto len {nullptr != tmp ? strlen(tmp) : 0};
                            offset += static_cast<DWORD>(len + sizeof(char));
                        }
                    }

                    if (offset < rawDataSize)
                    {
                        serialNumber = reinterpret_cast<const char*>(tmpBuffer.get() + offset);
                    }
                }
                else
                {
                    offset += header.FormattedAreaLength;

                    // Search for the end of the unformatted structure (\0\0)
                    while (offset + 1 < rawDataSize)
                    {
                        if (!(*(tmpBuffer.get() + offset)) && !(*(tmpBuffer.get() + offset + 1)))
                        {
                            offset += 2;
                            break;
                        }

                        offset++;
                    }
                }
            }
        }

        return serialNumber;
    }

    std::string buildTimestamp(const ULONGLONG time)
    {
        // Format of value is 18-digit LDAP/FILETIME timestamps.
        // 18-digit LDAP/FILETIME timestamps -> Epoch/Unix time
        // (value/10000000ULL) - 11644473600ULL
        const time_t epochTime {static_cast<long int>((time / 10000000ULL) - WINDOWS_UNIX_EPOCH_DIFF_SECONDS)};
        char formatString[20] = {0};

        tm utc_time;
        gmtime_s(&utc_time, &epochTime);

        std::strftime(formatString, sizeof(formatString), "%Y/%m/%d %H:%M:%S", &utc_time);
        return formatString;
    }
} // namespace Utils
