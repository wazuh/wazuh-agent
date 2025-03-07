#include "byteArrayHelper.hpp"

namespace Utils
{
    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-avoid-magic-numbers)
    int32_t toInt32BE(const uint8_t* bytes)
    {
        return static_cast<int32_t>(bytes[3]) | static_cast<int32_t>(bytes[2]) << 8 |
               static_cast<int32_t>(bytes[1]) << 16 | static_cast<int32_t>(bytes[0]) << 24;
    }

    int32_t toInt32LE(const uint8_t* bytes)
    {
        return static_cast<int32_t>(bytes[0]) | static_cast<int32_t>(bytes[1]) << 8 |
               static_cast<int32_t>(bytes[2]) << 16 | static_cast<int32_t>(bytes[3]) << 24;
    }

    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic, cppcoreguidelines-avoid-magic-numbers)
} // namespace Utils
