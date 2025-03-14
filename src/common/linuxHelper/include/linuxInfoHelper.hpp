#pragma once

#include <cstdint>
#include <unistd.h>

namespace Utils
{
    /// @brief Get system boot time in seconds since epoch
    /// @return uint64_t system boot time in seconds since epoch
    uint64_t getBootTime(void);

    /// @brief Get the number of clock ticks per second
    /// @return uint64_t number of clock ticks per second
    uint64_t getClockTick(void);

    /// @brief Convert a boot-relative time in ticks to seconds since epoch
    /// @param startTime boot-relative time in ticks
    /// @return uint64_t seconds since epoch
    uint64_t timeTick2unixTime(const uint64_t startTime);
} // namespace Utils
