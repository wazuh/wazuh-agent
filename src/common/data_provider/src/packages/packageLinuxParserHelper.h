/*
 * Wazuh SYSINFO
 * Copyright (C) 2015, Wazuh Inc.
 * January 28, 2021.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _PACKAGE_LINUX_PARSER_HELPER_H
#define _PACKAGE_LINUX_PARSER_HELPER_H

#include "sharedDefs.h"
#include "stringHelper.h"
#include <nlohmann/json.hpp>
#include "timeHelper.h"
#include "sharedDefs.h"

#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4505)
#endif

#include <sstream>

// Parse helpers for standard Linux packaging systems (rpm, dpkg, ...)
namespace PackageLinuxHelper
{

    static nlohmann::json parseDpkg(const std::vector<std::string>& entries)
    {
        std::map<std::string, std::string> info;
        nlohmann::json ret;

        for (const auto& entry : entries)
        {
            const auto pos{entry.find(":")};

            if (pos != std::string::npos)
            {
                const auto key{Utils::trim(entry.substr(0, pos))};
                const auto value{Utils::trim(entry.substr(pos + 1), " \n")};
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

            nlohmann::json priority {UNKNOWN_VALUE};
            nlohmann::json groups {UNKNOWN_VALUE};
            // The multiarch field won't have a default value
            nlohmann::json multiarch;
            nlohmann::json architecture {UNKNOWN_VALUE};
            nlohmann::json source {UNKNOWN_VALUE};
            nlohmann::json version {UNKNOWN_VALUE};
            nlohmann::json vendor {UNKNOWN_VALUE};
            nlohmann::json description {UNKNOWN_VALUE};
            int size                 { 0 };

            auto it{info.find("Priority")};

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
                size = stol(it->second) * 1024;
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

            ret["priority"]     = priority;
            ret["groups"]       = groups;
            ret["size"]         = size;
            ret["multiarch"]    = multiarch;
            ret["architecture"] = architecture;
            ret["source"]       = source;
            ret["version"]      = version;
            ret["format"]       = "deb";
            ret["location"]     = UNKNOWN_VALUE;
            ret["vendor"]       = vendor;
            ret["install_time"] = UNKNOWN_VALUE;
            ret["description"]  = description;
            ret["location"]     = UNKNOWN_VALUE;
        }

        return ret;
    }

    static nlohmann::json parseSnap(const nlohmann::json& info)
    {
        nlohmann::json ret;

        std::string name;
        std::string version;
        nlohmann::json vendor       { UNKNOWN_VALUE };
        nlohmann::json install_time { UNKNOWN_VALUE };
        nlohmann::json description  { UNKNOWN_VALUE };
        int         size         { 0 };
        bool        hasName      { false };
        bool        hasVersion   { false };

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
                size = data.get<int>();
            }
            else if (data.is_string())
            {
                const auto& stringData = data.get_ref<const std::string&>();

                if (stringData.length())
                {
                    try
                    {
                        size = std::stoi( stringData );
                    }
                    catch (const std::exception& e)
                    {
                        size = 0;
                    }
                }
            }
        }

        ret["name"]             = name;
        ret["location"]         = "/snap/" + name;
        ret["version"]          = version;
        ret["vendor"]           = vendor;
        ret["install_time"]     = install_time;
        ret["description"]      = description;
        ret["size"]             = size;
        ret["source"]           = "snapcraft";
        ret["format"]           = "snap";

        ret["priority"]         = UNKNOWN_VALUE;
        ret["multiarch"]        = UNKNOWN_VALUE;
        ret["architecture"]     = UNKNOWN_VALUE;
        ret["groups"]           = UNKNOWN_VALUE;

        return ret;
    }


};

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

#endif // _PACKAGE_LINUX_PARSER_HELPER_H
