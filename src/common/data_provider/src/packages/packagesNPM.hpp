#pragma once

#include "filesystem_utils.hpp"
#include "filesystem_wrapper.hpp"

#include "jsonIO.hpp"
#include "sharedDefs.h"
#include <nlohmann/json.hpp>

#include <filesystem>
#include <fstream>
#include <iostream>
#include <memory>
#include <set>

/// @brief NPM parser
template<typename TJsonReader = Utils::JsonIO<nlohmann::json>>
class NPM final : public TJsonReader
{
public:
    /// @brief NPM constructor
    NPM(std::unique_ptr<IFileSystemUtils> fsUtils = nullptr,
        std::unique_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr)
        : m_fsUtils(fsUtils ? std::move(fsUtils) : std::make_unique<file_system::FileSystemUtils>())
        , m_fileSystemWrapper(fileSystemWrapper ? std::move(fileSystemWrapper)
                                                : std::make_unique<file_system::FileSystemWrapper>())
    {
    }

    /// @brief NPM destructor
    ~NPM() = default;

    /// @brief Retrieves the NPM packages information
    /// @param osRootDirectories Paths to search for packages
    /// @param callback Callback function
    void getPackages(const std::set<std::string>& osRootDirectories, std::function<void(nlohmann::json&)> callback)
    {
        // Iterate over node_modules directories
        for (const auto& osRootDirectory : osRootDirectories)
        {
            try
            {
                std::deque<std::string> expandedPaths;

                // Expand paths
                m_fsUtils->expand_absolute_path(osRootDirectory, expandedPaths);
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
    /// @brief Parse NPM package
    /// @param folderPath Path to package folder
    /// @param callback Callback function
    void parsePackage(const std::filesystem::path& folderPath, std::function<void(nlohmann::json&)>& callback)
    {
        // Map to match fields
        static const std::map<std::string, std::string> NPM_FIELDS {
            {"name", "name"},
            {"version", "version"},
            {"description", "description"},
            {"homepage", "source"},
        };
        const auto path = folderPath / "package.json";

        try
        {
            if (m_fileSystemWrapper->exists(path))
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
            std::cerr << "Error reading NPM package: " << path.string() << ", " << e.what() << '\n';
        }
    }

    /// @brief Explore expanded paths
    /// @param expandedPaths Paths to explore
    /// @param callback Callback function
    void exploreExpandedPaths(const std::deque<std::string>& expandedPaths,
                              std::function<void(nlohmann::json&)> callback)
    {
        for (const auto& expandedPath : expandedPaths)
        {
            try
            {
                // Exist and is a directory
                const auto nodeModulesFolder {std::filesystem::path(expandedPath) / "node_modules"};

                if (m_fileSystemWrapper->exists(nodeModulesFolder))
                {
                    for (const auto& packageFolder : m_fileSystemWrapper->list_directory(nodeModulesFolder))
                    {
                        if (m_fileSystemWrapper->is_directory(packageFolder))
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
    std::unique_ptr<IFileSystemUtils> m_fsUtils;

    /// @brief Member to interact with the file system.
    std::unique_ptr<IFileSystemWrapper> m_fileSystemWrapper;
};
