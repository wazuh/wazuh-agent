#include <configuration_parser_utils.hpp>

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string>

namespace
{
    constexpr uint64_t A_SECOND_IN_MILLIS = 1000;
    constexpr uint64_t A_MINUTE_IN_MILLIS = 60 * A_SECOND_IN_MILLIS;
    constexpr uint64_t A_HOUR_IN_MILLIS = 60 * A_MINUTE_IN_MILLIS;
    constexpr uint64_t A_DAY_IN_MILLIS = 24 * A_HOUR_IN_MILLIS;

    constexpr uint64_t A_KB_IN_BYTES = 1000;
    constexpr uint64_t A_MB_IN_BYTES = 1000 * A_KB_IN_BYTES;
    constexpr uint64_t A_GB_IN_BYTES = 1000 * A_MB_IN_BYTES;

    uint64_t SafeMultiply(uint64_t value, uint64_t multiplier, const std::string& option)
    {
        if (multiplier != 0 && value > std::numeric_limits<uint64_t>::max() / multiplier)
        {
            throw std::out_of_range("Value overflows uint64_t: " + option);
        }
        return value * multiplier;
    }
} // namespace

std::time_t ParseTimeUnit(const std::string& option)
{
    std::string number;
    uint64_t multiplier = 1;

    if (option.ends_with("ms"))
    {
        number = option.substr(0, option.length() - 2);
    }
    else if (option.ends_with("s"))
    {
        number = option.substr(0, option.length() - 1);
        multiplier = A_SECOND_IN_MILLIS;
    }
    else if (option.ends_with("m"))
    {
        number = option.substr(0, option.length() - 1);
        multiplier = A_MINUTE_IN_MILLIS;
    }
    else if (option.ends_with("h"))
    {
        number = option.substr(0, option.length() - 1);
        multiplier = A_HOUR_IN_MILLIS;
    }
    else if (option.ends_with("d"))
    {
        number = option.substr(0, option.length() - 1);
        multiplier = A_DAY_IN_MILLIS;
    }
    else
    {
        // By default, assume seconds
        number = option;
        multiplier = A_SECOND_IN_MILLIS;
    }

    if (!std::all_of(number.begin(), number.end(), static_cast<int (*)(int)>(std::isdigit)))
    {
        throw std::invalid_argument("Invalid time unit: " + option);
    }
    const auto value = std::stoull(number);
    const auto result = SafeMultiply(value, multiplier, option);

    if (result > static_cast<uint64_t>(std::numeric_limits<std::time_t>::max()))
    {
        throw std::out_of_range("Value exceeds time_t range: " + option);
    }
    return static_cast<std::time_t>(result);
}

size_t ParseSizeUnit(const std::string& option)
{
    std::string number;
    unsigned int multiplier = 1;

    if (option.ends_with("K"))
    {
        number = option.substr(0, option.length() - 1);
        multiplier = A_KB_IN_BYTES;
    }
    else if (option.ends_with("KB"))
    {
        number = option.substr(0, option.length() - 2);
        multiplier = A_KB_IN_BYTES;
    }
    else if (option.ends_with("M"))
    {
        number = option.substr(0, option.length() - 1);
        multiplier = A_MB_IN_BYTES;
    }
    else if (option.ends_with("MB"))
    {
        number = option.substr(0, option.length() - 2);
        multiplier = A_MB_IN_BYTES;
    }
    else if (option.ends_with("G"))
    {
        number = option.substr(0, option.length() - 1);
        multiplier = A_GB_IN_BYTES;
    }
    else if (option.ends_with("GB"))
    {
        number = option.substr(0, option.length() - 2);
        multiplier = A_GB_IN_BYTES;
    }
    else if (option.ends_with("B"))
    {
        number = option.substr(0, option.length() - 1);
    }
    else
    {
        // By default, assume B
        number = option;
    }

    if (!std::all_of(number.begin(), number.end(), static_cast<int (*)(int)>(std::isdigit)))
    {
        throw std::invalid_argument("Invalid size unit: " + option);
    }

    const auto value = std::stoull(number);
    const auto result = SafeMultiply(value, multiplier, option);

    if (result > static_cast<uint64_t>(std::numeric_limits<size_t>::max()))
    {
        throw std::out_of_range("Value exceeds time_t range: " + option);
    }
    return static_cast<size_t>(result);
}
