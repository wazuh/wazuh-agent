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

#include <memory>
#include "filesystemHelper.h"
#include <filesystem.hpp>
#include <nlohmann/json.hpp>
#include "sharedDefs.h"
#include "utilsWrapperLinux.hpp"
#include "packageLinuxParserRpm.hpp"

/**
 * @brief Fills a JSON object with all available rpm-related information for legacy Linux.
 * @param callback Callback to be called for every single element being found
 */
void getRpmInfoLegacy(std::function<void(nlohmann::json&)> callback);

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

// Exception template
template <LinuxType linuxType>
class FactoryPackagesCreator final
{
    public:
        static void getPackages(std::function<void(nlohmann::json&)> /*callback*/)
        {
            throw std::runtime_error
            {
                "Error creating package data retriever."
            };
        }
};

// Standard template to extract package information in fully compatible Linux systems
template <>
class FactoryPackagesCreator<LinuxType::STANDARD> final
{
    public:
        static void getPackages(std::function<void(nlohmann::json&)> callback)
        {
            const auto fsWrapper = std::make_unique<filesystem::FileSystem>();
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

// Template to extract package information in partially incompatible Linux systems
template <>
class FactoryPackagesCreator<LinuxType::LEGACY> final
{
    public:
        static void getPackages(std::function<void(nlohmann::json&)> callback)
        {
            const auto fsWrapper = std::make_unique<filesystem::FileSystem>();
            if (fsWrapper->exists(RPM_PATH) && fsWrapper->is_directory(RPM_PATH))
            {
                getRpmInfoLegacy(callback);
            }
        }
};

#endif // _PACKAGE_LINUX_DATA_RETRIEVER_H
