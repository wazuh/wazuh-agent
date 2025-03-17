#pragma once

#include "filesystem_wrapper.hpp"
#include "packageLinuxParserRpm.hpp"
#include "sharedDefs.h"
#include "utilsWrapper.hpp"
#include <memory>
#include <nlohmann/json.hpp>

/// @brief Fills a JSON object with all available dpkg-related information
/// @param libPath  Path to dpkg's database directory
/// @param callback Callback to be called for every single element being found
void getDpkgInfo(const std::string& libPath, std::function<void(nlohmann::json&)> callback);

/// @brief Fills a JSON object with all available snap-related information
/// @param callback Callback to be called for every single element being found
void getSnapInfo(std::function<void(nlohmann::json&)> callback);

/// @brief Factory for Linux package information
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
