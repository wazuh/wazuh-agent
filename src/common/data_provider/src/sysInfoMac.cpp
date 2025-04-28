#include "cmdHelper.hpp"
#include "hardware/factoryHardwareFamilyCreator.h"
#include "hardware/hardwareWrapperImplMac.h"
#include "osPrimitivesImplMac.h"
#include "osinfo/sysOsParsers.h"
#include "packages/modernPackageDataRetriever.hpp"
#include "packages/packageFamilyDataAFactory.h"
#include "packages/packageMac.h"
#include "ports/portBSDWrapper.h"
#include "ports/portImpl.h"
#include "sqliteWrapper.hpp"
#include "stringHelper.hpp"
#include "sysInfo.hpp"
#include <filesystem_wrapper.hpp>
#include <grp.h>
#include <libproc.h>
#include <pwd.h>
#include <span>
#include <sqlite3.h>
#include <sys/proc.h>
#include <sys/proc_info.h>
#include <sys/sysctl.h>
#include <sys/utsname.h>

const std::string MAC_UTILITIES_PATH {"/Applications/Utilities"};
const std::string MACPORTS_DB_NAME {"registry.db"};
const std::string MACPORTS_QUERY {"SELECT name, version, date, location, archs FROM ports WHERE state = 'installed';"};
constexpr auto MAC_ROSETTA_DEFAULT_ARCH {"arm64"};

using ProcessTaskInfo = struct proc_taskallinfo;

static const std::vector<int> S_VALID_FD_SOCK = {{SOCKINFO_TCP, SOCKINFO_IN}};

static const std::map<std::string, int> S_MAP_PACKAGES_DIRECTORIES = {{"/Applications", PKG},
                                                                      {"/Applications/Utilities", PKG},
                                                                      {"/System/Applications", PKG},
                                                                      {"/System/Applications/Utilities", PKG},
                                                                      {"/System/Library/CoreServices", PKG},
                                                                      {"/private/var/db/receipts", RCP},
                                                                      {"/Library/Apple/System/Library/Receipts", RCP},
                                                                      {"/usr/local/Cellar", BREW},
                                                                      {"/opt/local/var/macports/registry", MACPORTS}};

static nlohmann::json GetProcessInfo(const ProcessTaskInfo& taskInfo, const pid_t pid)
{
    nlohmann::json jsProcessInfo {};
    jsProcessInfo["pid"] = std::to_string(pid);
    jsProcessInfo["name"] = taskInfo.pbsd.pbi_name;

    jsProcessInfo["state"] = UNKNOWN_VALUE;
    jsProcessInfo["ppid"] = taskInfo.pbsd.pbi_ppid;

    const auto eUser {getpwuid(taskInfo.pbsd.pbi_uid)};

    if (eUser)
    {
        jsProcessInfo["euser"] = eUser->pw_name;
    }

    const auto rUser {getpwuid(taskInfo.pbsd.pbi_ruid)};

    if (rUser)
    {
        jsProcessInfo["ruser"] = rUser->pw_name;
    }

    const auto rGroup {getgrgid(taskInfo.pbsd.pbi_rgid)};

    if (rGroup)
    {
        jsProcessInfo["rgroup"] = rGroup->gr_name;
    }

    jsProcessInfo["priority"] = taskInfo.ptinfo.pti_priority;
    jsProcessInfo["nice"] = taskInfo.pbsd.pbi_nice;
    jsProcessInfo["vm_size"] = taskInfo.ptinfo.pti_virtual_size / KByte;
    jsProcessInfo["start_time"] = taskInfo.pbsd.pbi_start_tvsec;
    return jsProcessInfo;
}

nlohmann::json SysInfo::getHardware() const
{
    nlohmann::json hardware;
    FactoryHardwareFamilyCreator<OSPlatformType::BSDBASED>::create(
        std::make_shared<OSHardwareWrapperMac<OsPrimitivesMac>>())
        ->buildHardwareData(hardware);
    return hardware;
}

static void GetPackagesFromPath(const std::string& pkgDirectory,
                                const int pkgType,
                                const std::function<void(nlohmann::json&)>& callback)
{
    if (MACPORTS == pkgType)
    {
        const auto fsWrapper = std::make_unique<file_system::FileSystemWrapper>();
        if (fsWrapper->exists(pkgDirectory + "/" + MACPORTS_DB_NAME) &&
            fsWrapper->is_regular_file(pkgDirectory + "/" + MACPORTS_DB_NAME))
        {
            try
            {
                std::shared_ptr<SQLiteLegacy::IConnection> sqliteConnection =
                    std::make_shared<SQLiteLegacy::Connection>(pkgDirectory + "/" + MACPORTS_DB_NAME);

                SQLiteLegacy::Statement stmt {sqliteConnection, MACPORTS_QUERY};

                const std::pair<SQLiteLegacy::IStatement&, const int&> pkgContext {
                    std::make_pair(std::ref(stmt), std::cref(pkgType))};

                while (SQLITE_ROW == stmt.step())
                {
                    try
                    {
                        nlohmann::json jsPackage;
                        FactoryPackageFamilyCreator<OSPlatformType::BSDBASED>::create(pkgContext)
                            ->buildPackageData(jsPackage);

                        if (!jsPackage.at("name").get_ref<const std::string&>().empty())
                        {
                            // Only return valid content packages
                            callback(jsPackage);
                        }
                    }
                    catch (const std::exception& e)
                    {
                        std::cerr << e.what() << '\n';
                    }
                }
            }
            catch (const std::exception& e)
            {
                std::cerr << e.what() << '\n';
            }
        }
    }
    else
    {
        const auto fsWrapper = std::make_unique<file_system::FileSystemWrapper>();
        std::vector<std::filesystem::path> packages;
        if (fsWrapper->exists(pkgDirectory) && fsWrapper->is_directory(pkgDirectory))
        {
            packages = fsWrapper->list_directory(pkgDirectory);
        }

        for (const auto& package : packages)
        {
            const auto packageName {package.filename().string()};
            if ((PKG == pkgType && Utils::endsWith(packageName, ".app")) ||
                (RCP == pkgType && Utils::endsWith(packageName, ".plist")))
            {
                try
                {
                    nlohmann::json jsPackage;
                    FactoryPackageFamilyCreator<OSPlatformType::BSDBASED>::create(
                        std::make_pair(PackageContext {pkgDirectory, packageName, ""}, pkgType))
                        ->buildPackageData(jsPackage);

                    if (!jsPackage.at("name").get_ref<const std::string&>().empty())
                    {
                        // Only return valid content packages
                        callback(jsPackage);
                    }
                }
                catch (const std::exception& e)
                {
                    std::cerr << e.what() << '\n';
                }
            }
            else if (BREW == pkgType)
            {
                if (!Utils::startsWith(packageName, "."))
                {
                    std::vector<std::filesystem::path> packageVersions;
                    std::filesystem::path packagePath {pkgDirectory + "/"};
                    packagePath += packageName;

                    if (fsWrapper->exists(packagePath) && fsWrapper->is_directory(packagePath))
                    {
                        packageVersions = fsWrapper->list_directory(packagePath);
                    }

                    for (const auto& version : packageVersions)
                    {
                        const auto versionFilename {version.filename().string()};
                        if (!Utils::startsWith(versionFilename, "."))
                        {
                            try
                            {
                                nlohmann::json jsPackage;
                                FactoryPackageFamilyCreator<OSPlatformType::BSDBASED>::create(
                                    std::make_pair(PackageContext {pkgDirectory, packageName, versionFilename},
                                                   pkgType))
                                    ->buildPackageData(jsPackage);

                                if (!jsPackage.at("name").get_ref<const std::string&>().empty())
                                {
                                    // Only return valid content packages
                                    callback(jsPackage);
                                }
                            }
                            catch (const std::exception& e)
                            {
                                std::cerr << e.what() << '\n';
                            }
                        }
                    }
                }
            }

            // else: invalid package
        }
    }
}

nlohmann::json SysInfo::getPackages() const
{
    nlohmann::json jsPackages;
    getPackages([&jsPackages](nlohmann::json& package) { jsPackages.push_back(package); });
    return jsPackages;
}

nlohmann::json SysInfo::getProcessesInfo() const
{
    nlohmann::json jsProcessesList {};

    getProcessesInfo(
        [&jsProcessesList](nlohmann::json& processInfo)
        {
            // Append the current json process object to the list of processes
            jsProcessesList.push_back(processInfo);
        });

    return jsProcessesList;
}

static bool IsRunningOnRosetta()
{

    /* Rosetta is a translation process that allows users to run
     *  apps that contain x86_64 instructions on Apple silicon.
     * The sysctl.proc_translated indicates if current process is being translated
     *   from x86_64 to arm64 (1) or not (0).
     * If sysctl.proc_translated flag cannot be found, the current process is
     *  nativally running on x86_64.
     * Ref: https://developer.apple.com/documentation/apple-silicon/about-the-rosetta-translation-environment
     */
    constexpr auto PROCESS_TRANSLATED {1};
    auto retVal {false};
    auto isTranslated {0};
    auto len {sizeof(isTranslated)};
    const auto result {sysctlbyname("sysctl.proc_translated", &isTranslated, &len, nullptr, 0)};

    if (result)
    {
        if (errno != ENOENT)
        {
            throw std::system_error {result, std::system_category(), "Error reading rosetta status."};
        }
    }
    else
    {
        retVal = PROCESS_TRANSLATED == isTranslated;
    }

    return retVal;
}

nlohmann::json SysInfo::getOsInfo() const
{
    nlohmann::json ret;

    struct utsname uts
    {
    };

    MacOsParser parser;
    parser.parseSwVersion(Utils::PipeOpen("sw_vers"), ret);
    parser.parseUname(Utils::PipeOpen("uname -r"), ret);

    if (!parser.parseSystemProfiler(Utils::PipeOpen("system_profiler SPSoftwareDataType"), ret))
    {
        ret["os_name"] = "macOS";
    }

    if (uname(&uts) >= 0)
    {
        ret["sysname"] = uts.sysname;
        ret["hostname"] = uts.nodename;
        ret["version"] = uts.version;
        ret["architecture"] = uts.machine;
        ret["release"] = uts.release;
    }

    if (IsRunningOnRosetta())
    {
        ret["architecture"] = MAC_ROSETTA_DEFAULT_ARCH;
    }

    return ret;
}

static void GetProcessesSocketFd(std::map<ProcessInfo, std::vector<std::shared_ptr<socket_fdinfo>>>& processSocket)
{
    int32_t maxProcess {0};
    auto maxProcessLen {sizeof(maxProcess)};

    if (!sysctlbyname("kern.maxproc", &maxProcess, &maxProcessLen, nullptr, 0))
    {
        auto pids {std::make_unique<pid_t[]>(static_cast<size_t>(maxProcess))};
        const auto processesCount {proc_listallpids(pids.get(), maxProcess)};

        for (auto i = 0; i < processesCount; ++i)
        {
            const auto pid {static_cast<pid_t>(pids[static_cast<size_t>(i)])};

            proc_bsdinfo processInformation {};

            if (proc_pidinfo(pid, PROC_PIDTBSDINFO, 0, &processInformation, PROC_PIDTBSDINFO_SIZE) != -1)
            {
                const std::string processName {processInformation.pbi_name};
                const ProcessInfo processData {pid, processName};

                const auto processFDBufferSize {proc_pidinfo(pid, PROC_PIDLISTFDS, 0, nullptr, 0)};

                if (processFDBufferSize != -1)
                {
                    auto processFDInformationBuffer {
                        std::make_unique<char[]>(static_cast<size_t>(processFDBufferSize))};

                    if (proc_pidinfo(pid, PROC_PIDLISTFDS, 0, processFDInformationBuffer.get(), processFDBufferSize) !=
                        -1)
                    {
                        auto processFDInformation =
                            std::span<proc_fdinfo>(std::bit_cast<proc_fdinfo*>(processFDInformationBuffer.get()),
                                                   static_cast<size_t>(processFDBufferSize) / PROC_PIDLISTFD_SIZE);

                        for (auto& fdInfo : processFDInformation)
                        {
                            if (PROX_FDTYPE_SOCKET == fdInfo.proc_fdtype)
                            {
                                auto socketInfo {std::make_shared<socket_fdinfo>()};

                                if (PROC_PIDFDSOCKETINFO_SIZE == proc_pidfdinfo(pid,
                                                                                fdInfo.proc_fd,
                                                                                PROC_PIDFDSOCKETINFO,
                                                                                socketInfo.get(),
                                                                                PROC_PIDFDSOCKETINFO_SIZE))
                                {
                                    if (socketInfo && std::find(S_VALID_FD_SOCK.begin(),
                                                                S_VALID_FD_SOCK.end(),
                                                                socketInfo->psi.soi_kind) != S_VALID_FD_SOCK.end())
                                    {
                                        processSocket[processData].push_back(socketInfo);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

nlohmann::json SysInfo::getPorts() const
{
    nlohmann::json ports;
    std::map<ProcessInfo, std::vector<std::shared_ptr<socket_fdinfo>>> fdMap;
    GetProcessesSocketFd(fdMap);

    for (const auto& processInfo : fdMap)
    {
        for (const auto& fdSocket : processInfo.second)
        {
            nlohmann::json port;
            std::make_unique<PortImpl>(std::make_shared<BSDPortWrapper>(processInfo.first, fdSocket))
                ->buildPortData(port);

            const auto portFound {std::find_if(ports.begin(),
                                               ports.end(),
                                               [&port](const auto& element)
                                               { return 0 == port.dump().compare(element.dump()); })};

            if (ports.end() == portFound)
            {
                ports.push_back(port);
            }
        }
    }

    return ports;
}

void SysInfo::getProcessesInfo(const std::function<void(nlohmann::json&)>& callback) const
{
    int32_t maxProc {};
    size_t len {sizeof(maxProc)};
    const auto ret {sysctlbyname("kern.maxproc", &maxProc, &len, nullptr, 0)};

    if (ret)
    {
        throw std::system_error {ret, std::system_category(), "Error reading kernel max processes."};
    }

    const auto spPids {std::make_unique<pid_t[]>(static_cast<size_t>(maxProc))};
    const auto processesCount {proc_listallpids(spPids.get(), maxProc)};

    for (int index = 0; index < processesCount; ++index)
    {
        ProcessTaskInfo taskInfo {};
        const auto pid {spPids.get()[index]};
        const auto sizeTask {proc_pidinfo(pid, PROC_PIDTASKALLINFO, 0, &taskInfo, PROC_PIDTASKALLINFO_SIZE)};

        if (PROC_PIDTASKALLINFO_SIZE == sizeTask)
        {
            auto processInfo = GetProcessInfo(taskInfo, pid);
            callback(processInfo);
        }
    }
}

void SysInfo::getPackages(const std::function<void(nlohmann::json&)>& callback) const
{
    const auto fsWrapper = std::make_unique<file_system::FileSystemWrapper>();
    for (const auto& packageDirectory : S_MAP_PACKAGES_DIRECTORIES)
    {
        const auto pkgDirectory {packageDirectory.first};

        if (fsWrapper->exists(pkgDirectory) && fsWrapper->is_directory(pkgDirectory))
        {
            GetPackagesFromPath(pkgDirectory, packageDirectory.second, callback);
        }
    }

    // Add all the unix default paths
    std::set<std::string> pypyMacOSPaths = {UNIX_PYPI_DEFAULT_BASE_DIRS.begin(), UNIX_PYPI_DEFAULT_BASE_DIRS.end()};

    // Add macOS specific paths
    pypyMacOSPaths.emplace("/Library/Python/*/*-packages");
    pypyMacOSPaths.emplace("/Users/*/Library/Python/*/lib/python/*-packages");
    pypyMacOSPaths.emplace("/Users/*/.pyenv/versions/*/lib/python*/*-packages");
    pypyMacOSPaths.emplace("/private/var/root/.pyenv/versions/*/lib/python*/*-packages");
    pypyMacOSPaths.emplace(
        "/Library/Developer/CommandLineTools/Library/Frameworks/Python3.framework/Versions/*/lib/python*/*-packages");
    pypyMacOSPaths.emplace("/System/Library/Frameworks/Python.framework/*-packages");
    pypyMacOSPaths.emplace("/opt/homebrew/lib/python*/*-packages");

    static const std::map<std::string, std::set<std::string>> searchPaths = {{"PYPI", pypyMacOSPaths},
                                                                             {"NPM", UNIX_NPM_DEFAULT_BASE_DIRS}};
    ModernFactoryPackagesCreator::getPackages(searchPaths, callback);
}

nlohmann::json SysInfo::getHotfixes() const
{
    // Currently not supported for this OS.
    return {};
}
