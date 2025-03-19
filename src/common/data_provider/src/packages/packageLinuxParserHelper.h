#pragma once

#include "sharedDefs.h"
#include "stringHelper.hpp"
#include "timeHelper.hpp"
#include <nlohmann/json.hpp>
#include <sstream>

// Parse helpers for standard Linux packaging systems (rpm, dpkg, ...)
namespace PackageLinuxHelper
{
    /// @brief Parse a dpkg database entry
    /// @param entries Vector of entries to parse
    /// @return Parsed information
    [[maybe_unused]] static nlohmann::json parseDpkg(const std::vector<std::string>& entries)
    {
        std::map<std::string, std::string> info;
        nlohmann::json ret;

        for (const auto& entry : entries)
        {
            const auto pos {entry.find(":")};

            if (pos != std::string::npos)
            {
                const auto key {Utils::Trim(entry.substr(0, pos))};
                const auto value {Utils::Trim(entry.substr(pos + 1), " \n")};
                info[key] = value;
            }
        }

        /*
           According to dpkg documentation, the status of the package consists in three fields separated by spaces:
           'SELECTION_STATE FLAG PACKAGE_STATE'.

           SELECTION_STATE: the desired action to take by the package manager. It could be 'install', 'hold',
                            'deinstall', 'purge', or 'unknown'.
           FLAG: indicates if the package requires a reinstall or if no issues were found. It could be 'ok',
                 or 'reinstreq'.
           PACKAGE_STATE: this is the real status of package at this moment. It could be 'not-installed',
                          'config-files', 'half-installed', 'unpacked', 'half-configured', 'triggers-awaited',
                          'triggers-pending', or 'installed'.

           We'll collect packages in any selection state, with 'ok' FLAG and 'installed' PACKAGE_STATE.
         */
        if (!info.empty() && info.at("Status").find("ok installed") != std::string::npos)
        {
            ret["name"] = info.at("Package");

            nlohmann::json priority = UNKNOWN_VALUE;
            nlohmann::json groups = UNKNOWN_VALUE;
            // The multiarch field won't have a default value
            nlohmann::json multiarch = UNKNOWN_VALUE;
            nlohmann::json architecture = EMPTY_VALUE;
            nlohmann::json source = UNKNOWN_VALUE;
            nlohmann::json version = EMPTY_VALUE;
            nlohmann::json vendor = UNKNOWN_VALUE;
            nlohmann::json description = UNKNOWN_VALUE;
            int64_t size {0};

            auto it {info.find("Priority")};

            if (it != info.end())
            {
                priority = it->second;
            }

            it = info.find("Section");

            if (it != info.end())
            {
                groups = it->second;
            }

            it = info.find("Installed-Size");

            if (it != info.end())
            {
                size = stoll(it->second) * 1024;
            }

            it = info.find("Multi-Arch");

            if (it != info.end())
            {
                multiarch = it->second;
            }

            it = info.find("Architecture");

            if (it != info.end())
            {
                architecture = it->second;
            }

            it = info.find("Source");

            if (it != info.end())
            {
                source = it->second;
            }

            it = info.find("Version");

            if (it != info.end())
            {
                version = it->second;
            }

            it = info.find("Maintainer");

            if (it != info.end())
            {
                vendor = it->second;
            }

            it = info.find("Description");

            if (it != info.end())
            {
                description = Utils::substrOnFirstOccurrence(it->second, "\n");
            }

            ret["priority"] = priority;
            ret["groups"] = groups;
            ret["size"] = size;
            ret["multiarch"] = multiarch;
            ret["architecture"] = architecture;
            ret["source"] = source;
            ret["version"] = version;
            ret["format"] = "deb";
            ret["location"] = EMPTY_VALUE;
            ret["vendor"] = vendor;
            ret["install_time"] = UNKNOWN_VALUE;
            ret["description"] = description;
        }

        return ret;
    }

    /// @brief Parse a snap package
    /// @param info Information to parse
    /// @return Parsed information
    [[maybe_unused]] static nlohmann::json ParseSnap(const nlohmann::json& info)
    {
        nlohmann::json ret;

        std::string name;
        std::string version;
        nlohmann::json vendor = UNKNOWN_VALUE;
        nlohmann::json install_time = UNKNOWN_VALUE;
        nlohmann::json description = UNKNOWN_VALUE;
        int64_t size {0};
        bool hasName {false};
        bool hasVersion {false};

        if (info.contains("name"))
        {
            name = info.at("name");

            if (name.length())
            {
                hasName = true;
            }
        }

        if (info.contains("version"))
        {
            version = info.at("version");

            if (version.length())
            {
                hasVersion = true;
            }
        }

        if (!(hasVersion && hasName))
        {
            ret.clear();
            return ret;
        }

        if (info.contains("publisher"))
        {
            auto& publisher = info.at("publisher");

            if (publisher.contains("display-name"))
            {
                vendor = publisher.at("display-name");
            }
        }

        if (info.contains("install-date"))
        {
            struct std::tm tm;
            std::istringstream iss(info.at("install-date").get<std::string>());
            iss >> std::get_time(&tm, "%Y-%m-%dT%H:%M:%S");
            std::ostringstream oss;
            oss << std::put_time(&tm, "%Y/%m/%d %H:%M:%S");
            install_time = oss.str();
        }

        if (info.contains("summary"))
        {
            description = info.at("summary");
        }

        if (info.contains("installed-size"))
        {
            auto& data = info.at("installed-size");

            if (data.is_number())
            {
                size = data.get<int64_t>();
            }
            else if (data.is_string())
            {
                const auto& stringData = data.get_ref<const std::string&>();

                if (stringData.length())
                {
                    try
                    {
                        size = std::stoll(stringData);
                    }
                    catch (const std::exception& e)
                    {
                        size = 0;
                    }
                }
            }
        }

        ret["name"] = name;
        ret["location"] = "/snap/" + name;
        ret["version"] = version;
        ret["vendor"] = vendor;
        ret["install_time"] = install_time;
        ret["description"] = description;
        ret["size"] = size;
        ret["source"] = "snapcraft";
        ret["format"] = "snap";

        ret["priority"] = UNKNOWN_VALUE;
        ret["multiarch"] = UNKNOWN_VALUE;
        ret["architecture"] = EMPTY_VALUE;
        ret["groups"] = UNKNOWN_VALUE;

        return ret;
    }

}; // namespace PackageLinuxHelper
