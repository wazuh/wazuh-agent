/*
 * Wazuh SYSINFO
 * Copyright (C) 2015, Wazuh Inc.
 * December 14, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _PACKAGE_INTERFACE_WRAPPER_H
#define _PACKAGE_INTERFACE_WRAPPER_H
#include "ipackageInterface.h"

class IPackageWrapper
{
    public:
        // LCOV_EXCL_START
        virtual ~IPackageWrapper() = default;
        // LCOV_EXCL_STOP
        virtual void name(nlohmann::json&) const = 0;
        virtual void version(nlohmann::json&) const = 0;
        virtual void groups(nlohmann::json&) const = 0;
        virtual void description(nlohmann::json&) const = 0;
        virtual void architecture(nlohmann::json&) const = 0;
        virtual void format(nlohmann::json&) const = 0;
        virtual void osPatch(nlohmann::json&) const = 0;
        virtual void source(nlohmann::json&) const = 0;
        virtual void location(nlohmann::json&) const = 0;
        virtual void priority(nlohmann::json&) const = 0;
        virtual void size(nlohmann::json&) const = 0;
        virtual void vendor(nlohmann::json&) const = 0;
        virtual void install_time(nlohmann::json&) const = 0;
        virtual void multiarch(nlohmann::json&) const = 0;
};
#endif // _PACKAGE_INTERFACE_WRAPPER_H
