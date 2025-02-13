#include <configuration_parser_utils.hpp>

#include <algorithm>
#include <stdexcept>

namespace
{
    constexpr unsigned int A_SECOND_IN_MILLIS = 1000;
    constexpr unsigned int A_MINUTE_IN_MILLIS = 60 * A_SECOND_IN_MILLIS;
    constexpr unsigned int A_HOUR_IN_MILLIS = 60 * A_MINUTE_IN_MILLIS;
    constexpr unsigned int A_DAY_IN_MILLIS = 24 * A_HOUR_IN_MILLIS;

    constexpr unsigned int A_KB_IN_BYTES = 1000;
    constexpr unsigned int A_MB_IN_BYTES = 1000 * A_KB_IN_BYTES;
    constexpr unsigned int A_GB_IN_BYTES = 1000 * A_MB_IN_BYTES;
} // namespace

std::time_t ParseTimeUnit(const std::string& option)
{
    std::string number;
    unsigned int multiplier = 1;

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

    return static_cast<std::time_t>(std::stoul(number) * multiplier);
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

    return static_cast<size_t>(std::stoul(number) * multiplier);
}
