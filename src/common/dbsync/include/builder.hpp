/*
 * Wazuh shared modules utils
 * Copyright (C) 2015-2021, Wazuh Inc.
 * January 19, 2022.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _BUILDER_PATTERN_HPP
#define _BUILDER_PATTERN_HPP

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4505)
#endif

/**
 * @brief This class provides a simple interface to construct an object using a Builder pattern.
 *
 * @tparam T Type of the object to be built.
 * @tparam Ts Arguments.
 */
template<typename T, class... Ts>
class Builder
{
public:
    /**
     * @brief This method is used to build an object.
     *
     * @param args Arguments.
     * @return T Object built.
     */
    static T builder(Ts... args)
    {
        return T(args...); // Default constructor
    }

    /**
     * @brief This method returns a reference to the object.
     * @return T Reference to the object.
     */
    T& build()
    {
        return static_cast<T&>(*this); // Return reference to self
    }
};

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // _BUILDER_PATTERN_HPP
