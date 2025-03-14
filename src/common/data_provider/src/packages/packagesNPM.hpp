/*
 * Wazuh data provider.
 * Copyright (C) 2015, Wazuh Inc.
 * July 11, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#pragma once

#include "filesystem_utils.hpp"
#include "ifilesystem_utils.hpp"
#include "filesystem_wrapper.hpp"

#include <nlohmann/json.hpp>
#include "jsonIO.hpp"
#include "sharedDefs.h"

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>

template<typename TFileSystem = filesystem::FileSystemWrapper, typename TJsonReader = JsonIO<nlohmann::json>>
class NPM final
    : public TFileSystem
    , public TJsonReader
{
    public:
        /// @brief NPM constructor
        NPM(std::shared_ptr<IFileSystemUtils> fsUtils = nullptr)
        : m_fsUtils(fsUtils ? fsUtils : std::make_shared<filesystem::FileSystemUtils>()) {}

        ~NPM() = default;

        void getPackages(const std::set<std::string>& osRootFolders, std::function<void(nlohmann::json&)> callback)
        {

            // Iterate over node_modules folders
            for (const auto& osRootFolder : osRootFolders)
            {
                try
                {
                    std::deque<std::string> expandedPaths;

                    // Expand paths
                    m_fsUtils->expand_absolute_path(osRootFolder, expandedPaths);
                    // Explore expanded paths
                    exploreExpandedPaths(expandedPaths, callback);
                }
                catch (const std::exception& e)
                {
                    // Ignore exception, continue with next folder
                    (void)e;
                }
            }
        }

    private:

        void parsePackage(const std::filesystem::path& folderPath, std::function<void(nlohmann::json&)>& callback)
        {
            // Map to match fields
            static const std::map<std::string, std::string> NPM_FIELDS
            {
                {"name", "name"},
                {"version", "version"},
                {"description", "description"},
                {"homepage", "source"},
            };
            const auto path = folderPath / "package.json";

            try
            {
                if (TFileSystem::exists(path))
                {
                    // Read json from filesystem path.
                    const auto packageJson = TJsonReader::readJson(path);
                    nlohmann::json packageInfo;

                    packageInfo["groups"] = UNKNOWN_VALUE;
                    packageInfo["description"] = UNKNOWN_VALUE;
                    packageInfo["architecture"] = EMPTY_VALUE;
                    packageInfo["format"] = "npm";
                    packageInfo["source"] = UNKNOWN_VALUE;
                    packageInfo["location"] = path.string();
                    packageInfo["priority"] = UNKNOWN_VALUE;
                    packageInfo["size"] = UNKNOWN_VALUE;
                    packageInfo["vendor"] = UNKNOWN_VALUE;
                    packageInfo["install_time"] = UNKNOWN_VALUE;
                    // The multiarch field won't have a default value

                    // Iterate over fields
                    for (const auto& [key, value] : NPM_FIELDS)
                    {
                        if (packageJson.contains(key) && packageJson.at(key).is_string())
                        {
                            packageInfo[value] = packageJson.at(key);
                        }
                    }

                    if (packageInfo.contains("name") && packageInfo.contains("version"))
                    {
                        callback(packageInfo);
                    }
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "Error reading NPM package: " << path.string() << ", " << e.what() << std::endl;
            }
        }

        void exploreExpandedPaths(const std::deque<std::string>& expandedPaths,
                                  std::function<void(nlohmann::json&)> callback)
        {
            for (const auto& expandedPath : expandedPaths)
            {
                try
                {
                    // Exist and is a directory
                    const auto nodeModulesFolder {std::filesystem::path(expandedPath) / "node_modules"};

                    if (TFileSystem::exists(nodeModulesFolder))
                    {
                        for (const auto& packageFolder : TFileSystem::list_directory(nodeModulesFolder))
                        {
                            if (TFileSystem::is_directory(packageFolder))
                            {
                                parsePackage(packageFolder, callback);
                            }
                        }
                    }
                }
                catch (const std::exception& e)
                {
                    // Ignore exception, continue with next folder
                    (void)e;
                }
            }
        }

        /// @brief Pointer to the file system utils
        std::shared_ptr<IFileSystemUtils> m_fsUtils;
};
