#pragma once

#include <ctime>
#include <string>

/// @brief Converts a time unit represented as a string to an time_t value (ms).
/// @param option A string representing a time unit.
/// @return The corresponding time_t value.
/// @throws std::invalid_argument if the string does not represent a valid time unit.
/// @details This function parses a string representing a time unit and returns the equivalent time_t
/// value. The time unit can be expressed in milliseconds (e.g. "1ms"), seconds (e.g. "1s"), minutes (e.g.
/// "1m"), hours (e.g. "1h"), or days (e.g. "1d"). If no unit is specified, the value is assumed to be in
/// seconds.
std::time_t ParseTimeUnit(const std::string& option);

/// @brief Converts a size unit represented as a string to an size_t value (B).
/// @param option A string representing a size unit.
/// @return The corresponding size_t value.
/// @throws std::invalid_argument if the string does not represent a valid size unit.
/// @details This function parses a string representing a size unit and returns the equivalent size_t
/// value. The size unit can be expressed in Bytes (e.g. "1B"), Mega bytes (e.g. "1M" or "1MB"), kilo bytes
/// (e.g. "1K" or "1KB"). If no unit is specified, the value is assumed to be in Bytes
size_t ParseSizeUnit(const std::string& option);
