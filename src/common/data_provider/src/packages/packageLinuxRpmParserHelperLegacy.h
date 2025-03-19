#pragma once

#include "sharedDefs.h"
#include "stringHelper.hpp"
#include <nlohmann/json.hpp>

namespace PackageLinuxHelper
{
    /// @brief Parse rpm package information
    /// @param packageInfo Package information
    /// @return Parsed package
    static nlohmann::json parseRpm(const std::string& packageInfo)
    {
        nlohmann::json ret;
        const auto fields {Utils::split(packageInfo, '\t')};
        constexpr auto DEFAULT_VALUE {"(none)"};

        if (RPMFields::RPM_FIELDS_SIZE <= fields.size())
        {
            std::string name {fields.at(RPMFields::RPM_FIELDS_NAME)};

            if (name.compare("gpg-pubkey") != 0 && !name.empty())
            {
                std::string size {fields.at(RPMFields::RPM_FIELDS_PACKAGE_SIZE)};
                std::string install_time {fields.at(RPMFields::RPM_FIELDS_INSTALLTIME)};
                std::string groups {fields.at(RPMFields::RPM_FIELDS_GROUPS)};
                std::string version {fields.at(RPMFields::RPM_FIELDS_VERSION)};
                std::string architecture {fields.at(RPMFields::RPM_FIELDS_ARCHITECTURE)};
                std::string vendor {fields.at(RPMFields::RPM_FIELDS_VENDOR)};
                std::string description {fields.at(RPMFields::RPM_FIELDS_SUMMARY)};

                std::string release {fields.at(RPMFields::RPM_FIELDS_RELEASE)};
                std::string epoch {fields.at(RPMFields::RPM_FIELDS_EPOCH)};

                if (!epoch.empty() && epoch.compare(DEFAULT_VALUE) != 0)
                {
                    version = epoch + ":" + version;
                }

                if (!release.empty() && release.compare(DEFAULT_VALUE) != 0)
                {
                    version += "-" + release;
                }

                ret["name"] = name;
                ret["size"] = size.empty() || size.compare(DEFAULT_VALUE) == 0 ? 0 : stoll(size);
                ret["install_time"] = install_time.empty() || install_time.compare(DEFAULT_VALUE) == 0
                                          ? UNKNOWN_VALUE
                                          : nlohmann::json(install_time);
                ret["location"] = EMPTY_VALUE;
                ret["groups"] =
                    groups.empty() || groups.compare(DEFAULT_VALUE) == 0 ? UNKNOWN_VALUE : nlohmann::json(groups);
                ret["version"] =
                    version.empty() || version.compare(DEFAULT_VALUE) == 0 ? EMPTY_VALUE : nlohmann::json(version);
                ret["priority"] = UNKNOWN_VALUE;
                ret["architecture"] = architecture.empty() || architecture.compare(DEFAULT_VALUE) == 0
                                          ? EMPTY_VALUE
                                          : nlohmann::json(architecture);
                ret["source"] = UNKNOWN_VALUE;
                ret["format"] = "rpm";
                ret["vendor"] =
                    vendor.empty() || vendor.compare(DEFAULT_VALUE) == 0 ? UNKNOWN_VALUE : nlohmann::json(vendor);
                ret["description"] = description.empty() || description.compare(DEFAULT_VALUE) == 0
                                         ? UNKNOWN_VALUE
                                         : nlohmann::json(description);
                // The multiarch field won't have a default value
            }
        }

        return ret;
    }

}; // namespace PackageLinuxHelper
