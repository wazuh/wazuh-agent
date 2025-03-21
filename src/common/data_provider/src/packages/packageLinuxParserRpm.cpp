#include "berkeleyRpmDbHelper.h"
#include "filesystem_wrapper.hpp"
#include "iberkeleyDbWrapper.h"
#include "packageLinuxDataRetriever.h"
#include "packageLinuxRpmParserHelper.h"
#include "packageLinuxRpmParserHelperLegacy.h"
#include "rpmlib.h"
#include "sharedDefs.h"
#include "stringHelper.hpp"
#include "utilsWrapper.hpp"

void GetRpmInfo(const std::function<void(nlohmann::json&)>& callback,
                std::unique_ptr<IFileSystemWrapper> fileSystemWrapper)
{
    const auto rpmDefaultQuery {[](const std::function<void(nlohmann::json&)>& cb)
                                {
                                    const auto rawRpmPackagesInfo {UtilsWrapper::exec(
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

    if (!fileSystemWrapper)
    {
        fileSystemWrapper = std::make_unique<file_system::FileSystemWrapper>();
    }

    if (!(fileSystemWrapper->exists(RPM_DATABASE) && fileSystemWrapper->is_regular_file(RPM_DATABASE)))
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
        catch (...) // NOLINT(bugprone-empty-catch)
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
        catch (...) // NOLINT(bugprone-empty-catch)
        {
            rpmDefaultQuery(callback);
        }
    }
}
