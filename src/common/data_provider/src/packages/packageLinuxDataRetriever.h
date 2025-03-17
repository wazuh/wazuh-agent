/*
 * Wazuh SYSINFO
 * Copyright (C) 2015, Wazuh Inc.
 * April 16, 2021.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _PACKAGE_LINUX_DATA_RETRIEVER_H
#define _PACKAGE_LINUX_DATA_RETRIEVER_H

#include "filesystem_wrapper.hpp"
#include "packageLinuxParserRpm.hpp"
#include "sharedDefs.h"
#include "utilsWrapperLinux.hpp"
#include <memory>
#include <nlohmann/json.hpp>

/**
 * @brief Fills a JSON object with all available dpkg-related information
 * @param libPath  Path to dpkg's database directory
 * @param callback Callback to be called for every single element being found
 */
void getDpkgInfo(const std::string& libPath, std::function<void(nlohmann::json&)> callback);

/**
 * @brief Fills a JSON object with all available snap-related information
 * @param callback Callback to be called for every single element being found
 */
void getSnapInfo(std::function<void(nlohmann::json&)> callback);

// Standard template to extract package information in fully compatible Linux systems
class FactoryPackagesCreator final
{
public:
    static void getPackages(std::function<void(nlohmann::json&)> callback)
    {
        const auto fsWrapper = std::make_unique<file_system::FileSystemWrapper>();
        if (fsWrapper->exists(DPKG_PATH) && fsWrapper->is_directory(DPKG_PATH))
        {
            getDpkgInfo(DPKG_STATUS_PATH, callback);
        }

        if (fsWrapper->exists(RPM_PATH) && fsWrapper->is_directory(RPM_PATH))
        {
            RPM<>().getRpmInfo(callback);
        }

        if (fsWrapper->exists(SNAP_PATH) && fsWrapper->is_directory(SNAP_PATH))
        {
            getSnapInfo(callback);
        }
    }
};

#endif // _PACKAGE_LINUX_DATA_RETRIEVER_H
