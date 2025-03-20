#pragma once

#include "file_io_utils.hpp"
#include "filesystem_utils.hpp"
#include "filesystem_wrapper.hpp"

#include "sharedDefs.h"
#include "stringHelper.hpp"
#include <nlohmann/json.hpp>

#include <iostream>
#include <memory>
#include <set>

const static std::map<std::string, std::string> FILE_MAPPING_PYPI {{"egg-info", "PKG-INFO"}, {"dist-info", "METADATA"}};

/// @brief PYPI parser
template<typename TFileIOUtils = file_io::FileIOUtils>
class PYPI final : public TFileIOUtils
{
public:
    /// @brief PYPI constructor
    PYPI(std::shared_ptr<IFileSystemUtils> fsUtils = nullptr,
         std::shared_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr)
        : m_fsUtils(fsUtils ? fsUtils : std::make_shared<file_system::FileSystemUtils>())
        , m_fileSystemWrapper(fileSystemWrapper ? fileSystemWrapper
                                                : std::make_shared<file_system::FileSystemWrapper>())
    {
    }

    /// @brief PYPI destructor
    ~PYPI() = default;

    /// @brief Retrieves the PYPI packages information
    /// @param osRootFolders Paths to search for packages
    /// @param callback Callback function
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
    /// @brief Parse the METADATA file
    /// @param path Path to the METADATA file
    /// @param callback Callback function
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
                                     [&packageInfo](const std::string& line) -> bool
                                     {
                                         const auto it {std::find_if(PYPI_FIELDS.begin(),
                                                                     PYPI_FIELDS.end(),
                                                                     [&line](const auto& element) {
                                                                         return Utils::startsWith(line, element.first);
                                                                     })};

                                         if (PYPI_FIELDS.end() != it)
                                         {
                                             const auto& [key, value] {*it};

                                             if (!packageInfo.contains(value))
                                             {
                                                 packageInfo[value] = Utils::Trim(line.substr(key.length()), "\r");
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

    /// @brief Find the correct path to the METADATA file
    /// @param path Path to the package folder
    /// @param callback Callback function
    void findCorrectPath(const std::filesystem::path& path, std::function<void(nlohmann::json&)> callback)
    {
        try
        {
            const auto filename = path.filename().string();

            for (const auto& [key, value] : FILE_MAPPING_PYPI)
            {
                if (filename.find(key) != std::string::npos)
                {
                    if (m_fileSystemWrapper->is_regular_file(path))
                    {
                        parseMetadata(path, callback);
                    }
                    else if (m_fileSystemWrapper->is_directory(path))
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
            std::cerr << "Error parsing PYPI package: " << path.string() << ", " << e.what() << '\n';
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
                if (m_fileSystemWrapper->exists(expandedPath) && m_fileSystemWrapper->is_directory(expandedPath))
                {
                    for (const std::filesystem::path& path : m_fileSystemWrapper->list_directory(expandedPath))
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

    /// @brief Member to interact with the file system.
    std::shared_ptr<IFileSystemWrapper> m_fileSystemWrapper;
};
