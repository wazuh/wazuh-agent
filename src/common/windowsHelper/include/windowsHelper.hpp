#pragma once

// clang-format off
#include <string>
#include <winsock2.h>
#include <windows.h>
// clang-format on

constexpr auto WINDOWS_UNIX_EPOCH_DIFF_SECONDS {11644473600ULL};
constexpr auto BASEBOARD_INFORMATION_TYPE {2};
constexpr auto REFERENCE_YEAR {1900};
constexpr auto HOURMINSEC_VALUE_SIZE {6};
constexpr auto INSTALLDATE_REGISTRY_VALUE_SIZE {8};

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4200)
#endif

/// @brief SMBIOS raw data
typedef struct RawSMBIOSData
{
    BYTE Used20CallingMethod;
    BYTE SMBIOSMajorVersion;
    BYTE SMBIOSMinorVersion;
    BYTE DmiRevision;
    DWORD Length;
    BYTE SMBIOSTableData[];
} RawSMBIOSData, *PRawSMBIOSData;

#ifdef _MSC_VER
#pragma warning(pop)
#endif

/// @brief SMBIOS structure header
typedef struct SMBIOSStructureHeader
{
    BYTE Type;
    BYTE FormattedAreaLength;
    WORD Handle;
} SMBIOSStructureHeader;

/// @brief SMBIOS baseboard info structure
typedef struct SMBIOSBaseboardInfoStructure
{
    BYTE Type;
    BYTE FormattedAreaLength;
    WORD Handle;
    BYTE Manufacturer;
    BYTE Product;
    BYTE Version;
    BYTE SerialNumber;
} SMBIOSBaseboardInfoStructure;

namespace Utils
{
    /// @brief Check if the OS is Windows Vista or later
    /// @return True if the OS is Windows Vista or later
    bool isVistaOrLater();

    /// @brief Normalize timestamp
    /// @param dateISO8601CalendarDateFormat Date in ISO 8601 calendar date format
    /// @param dateISO8601CombinedFormat Date in ISO 8601 combined date and time format
    /// @return Normalized timestamp
    std::string normalizeTimestamp(const std::string& dateISO8601CalendarDateFormat,
                                   const std::string& dateISO8601CombinedFormat);

    /// @brief Get serial number from SMBIOS
    /// @param rawData SMBIOS raw data
    /// @param rawDataSize SMBIOS raw data size
    /// @return Serial number
    std::string getSerialNumberFromSmbios(const BYTE* rawData, const DWORD rawDataSize);

    /// @brief Build timestamp
    /// @param time Timestamp to build
    /// @return Timestamp
    std::string buildTimestamp(const ULONGLONG time);
} // namespace Utils
