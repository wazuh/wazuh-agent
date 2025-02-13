/*
 * Wazuh SYSINFO
 * Copyright (C) 2015, Wazuh Inc.
 * December 14, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _BREW_WRAPPER_H
#define _BREW_WRAPPER_H

#include "ipackageWrapper.h"
#include "sharedDefs.h"
#include "stringHelper.h"
#include "file_io.hpp"

class BrewWrapper final : public IPackageWrapper
{
    public:
        explicit BrewWrapper(const PackageContext& ctx)
            : m_name{ctx.package}
            , m_version{Utils::splitIndex(ctx.version, '_', 0)}
            , m_groups{EMPTY_VALUE}
            , m_description {EMPTY_VALUE}
            , m_architecture{EMPTY_VALUE}
            , m_format{"pkg"}
            , m_source{"homebrew"}
            , m_location{ctx.filePath}
            , m_priority{EMPTY_VALUE}
            , m_vendor{EMPTY_VALUE}
            , m_installTime{EMPTY_VALUE}
        {
            const auto fileIoWrapper = std::make_unique<file_io::FileIO>();
            const auto rows { Utils::split(fileIoWrapper->getFileContent(ctx.filePath + "/" + ctx.package + "/" + ctx.version + "/.brew/" + ctx.package + ".rb"), '\n')};

            for (const auto& row : rows)
            {
                auto rowParsed { Utils::trim(row) };

                if (Utils::startsWith(rowParsed, "desc "))
                {
                    Utils::replaceFirst(rowParsed, "desc ", "");
                    Utils::replaceAll(rowParsed, "\"", "");
                    m_description = rowParsed;
                    break;
                }
            }

            /* Some brew packages have the version in the name separated by a '@'
              but we'll only remove the last occurrence if it matches with a version
              in case there is a '@' in the package name */
            const auto pos { m_name.rfind('@') };

            if (pos != std::string::npos)
            {
                if (std::isdigit(m_name[pos + 1]))
                {
                    m_name.resize(pos);
                }
            }
        }

        ~BrewWrapper() = default;

        void name(nlohmann::json& package) const override
        {
            package["name"] = m_name;
        }

        void version(nlohmann::json& package) const override
        {
            package["version"] = m_version;
        }

        void groups(nlohmann::json& package) const override
        {
            package["groups"] = UNKNOWN_VALUE;
        }

        void description(nlohmann::json& package) const override
        {
            package["description"] = m_description;
        }

        void architecture(nlohmann::json& package) const override
        {
            package["architecture"] = EMPTY_VALUE;
        }

        void format(nlohmann::json& package) const override
        {
            package["format"] = m_format;
        }

        void osPatch(nlohmann::json& package) const override
        {
            package["os_patch"] = UNKNOWN_VALUE;
        }

        void source(nlohmann::json& package) const override
        {
            package["source"] = m_source;
        }

        void location(nlohmann::json& package) const override
        {
            package["location"] = m_location;
        }

        void vendor(nlohmann::json& package) const override
        {
            package["vendor"] = UNKNOWN_VALUE;
        }

        void priority(nlohmann::json& package) const override
        {
            package["priority"] = UNKNOWN_VALUE;
        }

        void size(nlohmann::json& package) const override
        {
            package["size"] = UNKNOWN_VALUE;
        }

        void install_time(nlohmann::json& package) const override
        {
            package["install_time"] = UNKNOWN_VALUE;
        }

        void multiarch(nlohmann::json& package) const override
        {
            package["multiarch"] = UNKNOWN_VALUE;
        }

    private:
        std::string m_name;
        std::string m_version;
        std::string m_groups;
        std::string m_description;
        std::string m_architecture;
        const std::string m_format;
        std::string m_osPatch;
        const std::string m_source;
        const std::string m_location;
        std::string m_priority;
        std::string m_vendor;
        std::string m_installTime;
        std::string m_multiarch;
};


#endif //_BREW_WRAPPER_H
