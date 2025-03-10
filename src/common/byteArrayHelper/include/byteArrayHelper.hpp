#pragma once

#include <cstdint>
#include <string>

namespace Utils
{
    /// @brief Converts a byte array to an int32_t in big endian format
    /// @param bytes the byte array to convert
    /// @return the resulting int32_t
    int32_t toInt32BE(const uint8_t* bytes);

    /// @brief Converts a byte array to an int32_t in little endian format
    /// @param bytes the byte array to convert
    /// @return the resulting int32_t
    int32_t toInt32LE(const uint8_t* bytes);
} // namespace Utils
