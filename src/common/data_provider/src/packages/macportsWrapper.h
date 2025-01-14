/*
 * Wazuh SYSINFO
 * Copyright (C) 2015, Wazuh Inc.
 * February 06, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _MACPORTS_WRAPPER_H
#define _MACPORTS_WRAPPER_H

#include "ipackageWrapper.h"
#include "sqliteWrapperTemp.h"
#include "sharedDefs.h"

const std::map<std::string, int> columnIndexes
{
    {"name", 0},
    {"version", 1},
    {"date", 2},
    {"location", 3},
    {"archs", 4}
};

#define DATE_STR_SIZE 20

class MacportsWrapper final : public IPackageWrapper
{
    public:
        explicit MacportsWrapper(SQLiteLegacy::IStatement& stmt)
            : m_version{EMPTY_VALUE}
            , m_groups {EMPTY_VALUE}
            , m_description {EMPTY_VALUE}
            , m_architecture{EMPTY_VALUE}
            , m_format{"macports"}
            , m_osPatch {EMPTY_VALUE}
            , m_source{EMPTY_VALUE}
            , m_location{EMPTY_VALUE}
            , m_priority{EMPTY_VALUE}
            , m_vendor{EMPTY_VALUE}
            , m_installTime{EMPTY_VALUE}
        {
            getPkgData(stmt);
        }

        ~MacportsWrapper() = default;

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
            package["groups"] = EMPTY_VALUE;
        }
        void description(nlohmann::json& package) const override
        {
            package["description"] = EMPTY_VALUE;
        }
        void architecture(nlohmann::json& package) const override
        {
            package["architecture"] = m_architecture;
        }
        void format(nlohmann::json& package) const override
        {
            package["format"] = m_format;
        }
        void osPatch(nlohmann::json& package) const override
        {
            package["os_patch"] = EMPTY_VALUE;
        }
        void source(nlohmann::json& package) const override
        {
            package["source"] = EMPTY_VALUE;
        }
        void location(nlohmann::json& package) const override
        {
            package["location"] = m_location;
        }
        void vendor(nlohmann::json& package) const override
        {
            package["vendor"] = EMPTY_VALUE;
        }

        void priority(nlohmann::json& package) const override
        {
            package["priority"] = EMPTY_VALUE;
        }

        void size(nlohmann::json& package) const override
        {
            package["size"] = EMPTY_VALUE;
        }

        void install_time(nlohmann::json& package) const override
        {
            package["install_time"] = m_installTime;
        }

        void multiarch(nlohmann::json& package) const override
        {
            package["multiarch"] = EMPTY_VALUE;
        }
    private:
        void getPkgData(SQLiteLegacy::IStatement& stmt)
        {
            const int& columnsNumber = columnIndexes.size();

            if (stmt.columnsCount() == columnsNumber)
            {
                const auto& name {stmt.column(columnIndexes.at("name"))};
                const auto& version {stmt.column(columnIndexes.at("version"))};
                const auto& date {stmt.column(columnIndexes.at("date"))};
                const auto& location {stmt.column(columnIndexes.at("location"))};
                const auto& archs {stmt.column(columnIndexes.at("archs"))};

                if (name->hasValue())
                {
                    m_name = name->value(std::string {});
                }

                if (version->hasValue())
                {
                    const auto versionStr = version->value(std::string{});

                    if (!versionStr.empty())
                    {
                        m_version = versionStr;
                    }
                }

                if (date->hasValue())
                {
                    char formattedTime[DATE_STR_SIZE] {0};
                    const long epochTime = date->value(std::int64_t {});
                    std::strftime(formattedTime, sizeof(formattedTime), "%Y/%m/%d %H:%M:%S", std::localtime(&epochTime));
                    m_installTime = formattedTime;
                }

                if (location->hasValue())
                {
                    const auto locationStr = location->value(std::string {});

                    if (!locationStr.empty())
                    {
                        m_location = locationStr;
                    }
                }

                if (archs->hasValue())
                {
                    const auto archsStr = archs->value(std::string {});

                    if (!archsStr.empty())
                    {
                        m_architecture = archsStr;
                    }
                }
            }
        }

        std::string m_name;
        std::string m_version;
        std::string m_groups;
        std::string m_description;
        std::string m_architecture;
        const std::string m_format;
        std::string m_osPatch;
        std::string m_source;
        std::string m_location;
        std::string m_multiarch;
        std::string m_priority;
        std::string m_vendor;
        std::string m_installTime;
};

# endif // _MACPORTS_WRAPPER_H
