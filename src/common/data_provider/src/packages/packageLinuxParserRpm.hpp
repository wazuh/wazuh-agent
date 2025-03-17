/*
 * Wazuh SysInfo
 * Copyright (C) 2015, Wazuh Inc.
 * December 22, 2021.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation
 */

#ifndef _PACKAGES_LINUX_PARSER_RPM__HPP
#define _PACKAGES_LINUX_PARSER_RPM__HPP

#include "berkeleyRpmDbHelper.h"
#include "filesystem_wrapper.hpp"
#include "iberkeleyDbWrapper.h"
#include "packageLinuxDataRetriever.h"
#include "packageLinuxRpmParserHelper.h"
#include "packageLinuxRpmParserHelperLegacy.h"
#include "rpmlib.h"
#include "sharedDefs.h"
#include "stringHelper.hpp"
#include "utilsWrapperLinux.hpp"

template<typename TFileSystem = file_system::FileSystemWrapper>
class RPM final : public TFileSystem
{
public:
    /**
     * @brief Fills a JSON object with all available rpm-related information
     * @param callback Callback to be called for every single element being found
     */
    void getRpmInfo(std::function<void(nlohmann::json&)> callback)
    {

        const auto rpmDefaultQuery {[](std::function<void(nlohmann::json&)> cb)
                                    {
                                        const auto rawRpmPackagesInfo {UtilsWrapperLinux::exec(
                                            "rpm -qa --qf "
                                            "'%{name}\t%{arch}\t%{summary}\t%{size}\t%{epoch}\t%{release}\t%{version}"
                                            "\t%{vendor}\t%{installtime:date}\t%{group}\t\n'")};

                                        if (!rawRpmPackagesInfo.empty())
                                        {
                                            const auto rows {Utils::split(rawRpmPackagesInfo, '\n')};

                                            for (const auto& row : rows)
                                            {
                                                auto package = PackageLinuxHelper::parseRpm(row);

                                                if (!package.empty())
                                                {
                                                    cb(package);
                                                }
                                            }
                                        }
                                    }};

        if (!(TFileSystem::exists(RPM_DATABASE) && TFileSystem::is_regular_file(RPM_DATABASE)))
        {
            // We are probably using RPM >= 4.16 – get the packages from librpm.
            try
            {
                RpmPackageManager rpm {std::make_shared<RpmLib>()};

                for (const auto& p : rpm)
                {
                    auto packageJson = PackageLinuxHelper::parseRpm(p);

                    if (!packageJson.empty())
                    {
                        callback(packageJson);
                    }
                }
            }
            catch (...)
            {
                rpmDefaultQuery(callback);
            }
        }
        else
        {
            try
            {
                BerkeleyRpmDBReader db {std::make_shared<BerkeleyDbWrapper>(RPM_DATABASE)};
                auto row = db.getNext();

                // Get the packages from the Berkeley DB.
                while (!row.empty())
                {
                    auto package = PackageLinuxHelper::parseRpm(row);

                    if (!package.empty())
                    {
                        callback(package);
                    }

                    row = db.getNext();
                }
            }
            catch (...)
            {
                rpmDefaultQuery(callback);
            }
        }
    }
};

#endif // _PACKAGES_LINUX_PARSER_RPM__HPP
