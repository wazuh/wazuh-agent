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

#include "file_io_utils.hpp"
#include "filesystem_utils.hpp"
#include "filesystem_wrapper.hpp"

#include <nlohmann/json.hpp>
#include "sharedDefs.h"
#include "stringHelper.h"

#include <iostream>
#include <memory>
#include <set>

const static std::map<std::string, std::string> FILE_MAPPING_PYPI {{"egg-info", "PKG-INFO"}, {"dist-info", "METADATA"}};

template<typename TFileSystem = filesystem::FileSystemWrapper, typename TFileIOUtils = file_io::FileIOUtils>
class PYPI final : public TFileSystem, public TFileIOUtils
{
    public:
        /// @brief PYPI constructor
        PYPI(std::shared_ptr<IFileSystemUtils> fsUtils = nullptr)
        : m_fsUtils(fsUtils ? fsUtils : std::make_shared<filesystem::FileSystemUtils>()) {}

        void getPackages(const std::set<std::string>& osRootFolders, std::function<void(nlohmann::json&)> callback)
        {

            for (const auto& osFolder : osRootFolders)
            {
                std::deque<std::string> expandedPaths;

                try
                {
                    // Expand paths
                    m_fsUtils->expand_absolute_path(osFolder, expandedPaths);
                    // Explore expanded paths
                    exploreExpandedPaths(expandedPaths, callback);
                }
                catch (const std::exception&)
                {
                    // Do nothing, continue with the next path
                }
            }
        }

    private:
        void parseMetadata(const std::filesystem::path& path, std::function<void(nlohmann::json&)>& callback)
        {
            // Map to match fields
            static const std::map<std::string, std::string> PYPI_FIELDS {{"Name: ", "name"},
                {"Version: ", "version"},
                {"Summary: ", "description"},
                {"Home-page: ", "source"},
                {"Author: ", "vendor"}};

            // Parse the METADATA file
            nlohmann::json packageInfo;

            packageInfo["groups"] = UNKNOWN_VALUE;
            packageInfo["description"] = UNKNOWN_VALUE;
            packageInfo["architecture"] = EMPTY_VALUE;
            packageInfo["format"] = "pypi";
            packageInfo["source"] = UNKNOWN_VALUE;
            packageInfo["location"] = path.string();
            packageInfo["priority"] = UNKNOWN_VALUE;
            packageInfo["size"] = UNKNOWN_VALUE;
            packageInfo["vendor"] = UNKNOWN_VALUE;
            packageInfo["install_time"] = UNKNOWN_VALUE;
            // The multiarch field won't have a default value

            TFileIOUtils::readLineByLine(path,
                                    [&packageInfo](const std::string & line) -> bool
            {
                const auto it {
                    std::find_if(PYPI_FIELDS.begin(),
                                 PYPI_FIELDS.end(),
                                 [&line](const auto & element)
                    {
                        return Utils::startsWith(line, element.first);
                    })};

                if (PYPI_FIELDS.end() != it)
                {
                    const auto& [key, value] {*it};

                    if (!packageInfo.contains(value))
                    {
                        packageInfo[value] = Utils::trim(line.substr(key.length()), "\r");
                    }
                }
                return true;
            });

            // Check if we have a name and version
            if (packageInfo.contains("name") && packageInfo.contains("version"))
            {
                callback(packageInfo);
            }
        }

        void findCorrectPath(const std::filesystem::path& path, std::function<void(nlohmann::json&)> callback)
        {
            try
            {
                const auto filename = path.filename().string();

                for (const auto& [key, value] : FILE_MAPPING_PYPI)
                {
                    if (filename.find(key) != std::string::npos)
                    {
                        if (TFileSystem::is_regular_file(path))
                        {
                            parseMetadata(path, callback);
                        }
                        else if (TFileSystem::is_directory(path))
                        {
                            parseMetadata(path / value, callback);
                        }
                        else
                        {
                            // Do nothing
                        }
                    }
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << "Error parsing PYPI package: " << path.string() << ", " << e.what() << std::endl;
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
                    if (TFileSystem::exists(expandedPath) && TFileSystem::is_directory(expandedPath))
                    {
                        for (const std::filesystem::path& path : TFileSystem::list_directory(expandedPath))
                        {
                            findCorrectPath(path, callback);
                        }
                    }
                }
                catch (const std::exception&)
                {
                    // Do nothing, continue with the next path
                }
            }
        }

        /// @brief Pointer to the file system utils
        std::shared_ptr<IFileSystemUtils> m_fsUtils;
};
