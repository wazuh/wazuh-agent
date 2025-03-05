/* Copyright (C) 2015, Wazuh Inc.
 * All rights reserved.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation
 */

#include "berkeleyRpmDbHelper.h"
#include "iberkeleyDbWrapper.h"
#include "packageLinuxDataRetriever.h"
#include "packageLinuxRpmParserHelperLegacy.h"
#include "sharedDefs.h"

void getRpmInfoLegacy(std::function<void(nlohmann::json&)> callback)
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
