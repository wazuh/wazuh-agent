/*
 * Wazuh shared modules utils
 * Copyright (C) 2015, Wazuh Inc.
 * March 15, 2021.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _BYTE_ARRAY_HELPER_H
#define _BYTE_ARRAY_HELPER_H

#include <string>

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4505)
#endif

namespace Utils
{
    static int32_t toInt32BE(const uint8_t* bytes)
    {
        return static_cast<int32_t>(bytes[3]) | static_cast<int32_t>(bytes[2]) << 8 |
               static_cast<int32_t>(bytes[1]) << 16 | static_cast<int32_t>(bytes[0]) << 24;
    }

    static int32_t toInt32LE(const uint8_t* bytes)
    {
        return static_cast<int32_t>(bytes[0]) | static_cast<int32_t>(bytes[1]) << 8 |
               static_cast<int32_t>(bytes[2]) << 16 | static_cast<int32_t>(bytes[3]) << 24;
    }
} // namespace Utils

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // _BYTE_ARRAY_HELPER_H
