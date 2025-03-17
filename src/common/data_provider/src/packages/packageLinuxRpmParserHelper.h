#pragma once

#include "rpmPackageManager.h"
#include "sharedDefs.h"
#include <nlohmann/json.hpp>

// Parse helpers for standard Linux packaging systems (rpm, dpkg, ...)
namespace PackageLinuxHelper
{
    /// @brief Parse a rpm database entry
    /// @param package Package to parse
    /// @return Parsed package
    static nlohmann::json parseRpm(const RpmPackageManager::Package& package)
    {
        nlohmann::json ret;
        auto version {package.version};

        if (package.epoch)
        {
            version = std::to_string(package.epoch) + ":" + version;
        }

        if (!package.release.empty())
        {
            version += "-" + package.release;
        }

        if (package.name.compare("gpg-pubkey") != 0 && !package.name.empty())
        {
            ret["name"] = package.name;
            ret["size"] = package.size;
            ret["install_time"] = package.installTime;
            ret["location"] = EMPTY_VALUE;
            ret["groups"] = package.group;
            ret["version"] = version;
            ret["priority"] = UNKNOWN_VALUE;
            ret["architecture"] = package.architecture;
            ret["source"] = UNKNOWN_VALUE;
            ret["format"] = "rpm";
            ret["vendor"] = package.vendor.empty() ? UNKNOWN_VALUE : package.vendor;
            ret["description"] = package.description;
            // The multiarch field won't have a default value
        }

        return ret;
    }

}; // namespace PackageLinuxHelper
