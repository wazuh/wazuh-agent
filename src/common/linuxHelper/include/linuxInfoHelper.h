/*
 * Wazuh shared modules utils
 * Copyright (C) 2015, Wazuh Inc.
 * December 10, 2021.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef LINUXINFO_HELPER_H
#define LINUXINFO_HELPER_H

#include <vector>
#include <cstdint>
#include <string>
#include <unistd.h>

#include "file_io.hpp"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4505)
#endif

namespace Utils
{
    /**
     * @brief Get system boot time in seconds since epoch
     *
     * @return uint64_t system boot time in seconds since epoch
     */
    static uint64_t getBootTime(void)
    {
        static uint64_t btime;

        try
        {
            if (0UL == btime)
            {
                const std::string key {"btime "};
                const auto fileIoWrapper = std::make_unique<file_io::FileIO>();
                const auto file { fileIoWrapper->getFileContent("/proc/stat") };

                btime = std::stoull(file.substr(file.find(key) + key.length()));
            }
        }
        // LCOV_EXCL_START
        catch (...)
        {}

        // LCOV_EXCL_STOP

        return btime;
    }

    /**
     * @brief Get the number of clock ticks per second
     *
     * @return uint64_t number of clock ticks per second
     */
    static uint64_t getClockTick(void)
    {
        static uint64_t tick { static_cast<uint64_t>(sysconf(_SC_CLK_TCK)) };

        return tick;
    }

    /**
     * @brief Convert a boot-relative time in ticks to seconds since epoch
     *
     * @param startTime boot-relative time in ticks
     * @return uint64_t seconds since epoch
     */
    static uint64_t timeTick2unixTime(const uint64_t startTime)
    {
        return (startTime / getClockTick()) + getBootTime();
    }
}

#endif // LINUXINFO_HELPER_H
