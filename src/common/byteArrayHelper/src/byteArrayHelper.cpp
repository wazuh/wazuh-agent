#include "byteArrayHelper.hpp"

namespace Utils
{
    /// @brief Bytes shift
    enum BytesShift : uint8_t
    {
        THREE_BYTE = 24,
        TWO_BYTE = 16,
        ONE_BYTE = 8,
        ZERO_BYTE = 0
    };

    // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
    int32_t toInt32BE(const uint8_t* bytes)
    {
        return static_cast<int32_t>(bytes[3]) << ZERO_BYTE | static_cast<int32_t>(bytes[2]) << ONE_BYTE |
               static_cast<int32_t>(bytes[1]) << TWO_BYTE | static_cast<int32_t>(bytes[0]) << THREE_BYTE;
    }

    int32_t toInt32LE(const uint8_t* bytes)
    {
        return static_cast<int32_t>(bytes[0]) << ZERO_BYTE | static_cast<int32_t>(bytes[1]) << ONE_BYTE |
               static_cast<int32_t>(bytes[2]) << TWO_BYTE | static_cast<int32_t>(bytes[3]) << THREE_BYTE;
    }

    // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)
} // namespace Utils
