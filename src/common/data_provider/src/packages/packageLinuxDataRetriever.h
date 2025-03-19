#pragma once

#include "filesystem_wrapper.hpp"
#include "sharedDefs.h"
#include "utilsWrapper.hpp"
#include <memory>
#include <nlohmann/json.hpp>

/// @brief Fills a JSON object with all available dpkg-related information
/// @param fileName Path to dpkg's database directory
/// @param callback Callback to be called for every single element being found
void GetDpkgInfo(const std::string& fileName, const std::function<void(nlohmann::json&)>& callback);

/// @brief Fills a JSON object with all available snap-related information
/// @param callback Callback to be called for every single element being found
void GetSnapInfo(const std::function<void(nlohmann::json&)>& callback);

/**
 * @brief Fills a JSON object with all available rpm-related information
 * @param callback Callback to be called for every single element being found
 * @param fileSystemWrapper An optional filesystem wrapper. If nullptr, it will use FileSystemWrapper.
 */
void GetRpmInfo(const std::function<void(nlohmann::json&)>& callback,
                std::shared_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr);

class FactoryPackagesCreator final
{
public:
    /// @brief Retrieves the Linux package information
    /// @param callback Callback function
    static void getPackages(std::function<void(nlohmann::json&)> callback)
    {
        const auto fsWrapper = std::make_unique<file_system::FileSystemWrapper>();
        if (fsWrapper->exists(DPKG_PATH) && fsWrapper->is_directory(DPKG_PATH))
        {
            GetDpkgInfo(DPKG_STATUS_PATH, callback);
        }

        if (fsWrapper->exists(RPM_PATH) && fsWrapper->is_directory(RPM_PATH))
        {
            GetRpmInfo(callback);
        }

        if (fsWrapper->exists(SNAP_PATH) && fsWrapper->is_directory(SNAP_PATH))
        {
            GetSnapInfo(callback);
        }
    }
};
