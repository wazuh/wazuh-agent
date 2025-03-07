#pragma once

#include <cstdint>
#include <string>

namespace Utils
{
    int32_t toInt32BE(const uint8_t* bytes);

    int32_t toInt32LE(const uint8_t* bytes);
} // namespace Utils
