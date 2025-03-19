#include "cmdHelper.hpp"
#include "encodingWindowsHelper.hpp"
#include "network/networkFamilyDataAFactory.h"
#include "network/networkWindowsWrapper.h"
#include "networkWindowsHelper.hpp"
#include "osinfo/sysOsInfoWin.h"
#include "packages/appxWindowsWrapper.h"
#include "packages/modernPackageDataRetriever.hpp"
#include "packages/packagesNPM.hpp"
#include "packages/packagesPYPI.hpp"
#include "packages/packagesWindows.h"
#include "packages/packagesWindowsParserHelper.h"
#include "ports/portImpl.h"
#include "ports/portWindowsWrapper.h"
#include "registryHelper.hpp"
#include "stringHelper.hpp"
#include "sysInfo.hpp"
#include "sysinfoapi.h"
#include "timeHelper.hpp"
#include "windowsHelper.hpp"
#include <iphlpapi.h>
#include <list>
#include <memory>
#include <netioapi.h>
#include <ntstatus.h>
#include <psapi.h>
#include <set>
#include <system_error>
#include <tlhelp32.h>
#include <winternl.h>
#include <ws2tcpip.h>

constexpr auto CENTRAL_PROCESSOR_REGISTRY {"HARDWARE\\DESCRIPTION\\System\\CentralProcessor\\0"};
const std::string UNINSTALL_REGISTRY {"SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall"};
constexpr auto SYSTEM_IDLE_PROCESS_NAME {"System Idle Process"};
constexpr auto SYSTEM_PROCESS_NAME {"System"};

constexpr auto OS_MAXSTR {65536};
constexpr auto OS_MAXSTR_HALF {32768};

static const std::map<std::string, DWORD> gs_firmwareTableProviderSignature {
    {"ACPI", 0x41435049}, {"FIRM", 0x4649524D}, {"RSMB", 0x52534D42}};

/// @brief Windows process information
class SysInfoProcess final
{
public:
    /// @brief Constructor
    /// @param pId Process ID
    /// @param processHandle Process handle
    SysInfoProcess(const DWORD pId, const HANDLE processHandle)
        : m_pId {pId}
        , m_hProcess {processHandle}
        , m_creationTime {}
        , m_kernelModeTime {}
        , m_userModeTime {}
        , m_pageFileUsage {}
        , m_virtualSize {}
    {
        setProcessTimes();
        setProcessMemInfo();
    }

    /// @brief Default destructor
    ~SysInfoProcess() = default;

    /// @brief Returns the executable path
    /// @return Executable path
    std::string cmd()
    {
        std::string ret;
        const auto spReadBuff {std::make_unique<char[]>(OS_MAXSTR)};

        // Get full Windows kernel path for the process
        if (spReadBuff && GetProcessImageFileName(m_hProcess, spReadBuff.get(), OS_MAXSTR))
        {
            // Convert Windows kernel path to a valid Win32 filepath
            // E.g.: "\Device\HarddiskVolume1\Windows\system32\notepad.exe" -> "C:\Windows\system32\notepad.exe"
            ntPath2Win32Path(spReadBuff.get(), ret);
        }

        // else: Unable to retrieve executable path from current process.
        return ret;
    }

    /// @brief Returns the creation time
    /// @return Creation time
    ULONGLONG creationTime() const
    {
        // convert Win32 Epoch(1 January 1601 00:00:00) to Unix Epoch(1 January 1970 00:00:00)
        return m_creationTime.QuadPart - WINDOWS_UNIX_EPOCH_DIFF_SECONDS;
    }

    /// @brief Returns the kernel mode time
    /// @return Kernel mode time
    ULONGLONG kernelModeTime() const
    {
        return m_kernelModeTime.QuadPart;
    }

    /// @brief Returns the user mode time
    /// @return User mode time
    ULONGLONG userModeTime() const
    {
        return m_userModeTime.QuadPart;
    }

    /// @brief Returns the page file usage
    /// @return Page file usage
    DWORD pageFileUsage() const
    {
        return m_pageFileUsage;
    }

    /// @brief Returns the virtual size
    /// @return Virtual size
    DWORD virtualSize() const
    {
        return m_virtualSize;
    }

    /// @brief Returns the session ID
    /// @return Session ID
    DWORD sessionId() const
    {
        DWORD ret {};

        if (!ProcessIdToSessionId(m_pId, &ret))
        {
            // Unable to retrieve session ID from current process.
        }

        return ret;
    }

private:
    using SystemDrivesMap = std::map<std::string, std::string>;

    /// @brief Sets the process times
    void setProcessTimes()
    {
        constexpr auto TO_SECONDS_VALUE {10000000ULL};
        FILETIME lpCreationTime {};
        FILETIME lpExitTime {};
        FILETIME lpKernelTime {};
        FILETIME lpUserTime {};

        if (GetProcessTimes(m_hProcess, &lpCreationTime, &lpExitTime, &lpKernelTime, &lpUserTime))
        {
            // Copy the kernel mode filetime high and low parts and convert it to seconds
            m_kernelModeTime.LowPart = lpKernelTime.dwLowDateTime;
            m_kernelModeTime.HighPart = lpKernelTime.dwHighDateTime;
            m_kernelModeTime.QuadPart /= TO_SECONDS_VALUE;

            // Copy the user mode filetime high and low parts and convert it to seconds
            m_userModeTime.LowPart = lpUserTime.dwLowDateTime;
            m_userModeTime.HighPart = lpUserTime.dwHighDateTime;
            m_userModeTime.QuadPart /= TO_SECONDS_VALUE;

            // Copy the creation filetime high and low parts and convert it to seconds
            m_creationTime.LowPart = lpCreationTime.dwLowDateTime;
            m_creationTime.HighPart = lpCreationTime.dwHighDateTime;
            m_creationTime.QuadPart /= TO_SECONDS_VALUE;
        }

        // else: Unable to retrieve kernel mode and user mode times from current process.
    }

    /// @brief Sets the process memory info
    void setProcessMemInfo()
    {
        PROCESS_MEMORY_COUNTERS pMemCounters {};

        // Get page file usage and virtual size
        // Reference: https://stackoverflow.com/a/1986486
        if (GetProcessMemoryInfo(m_hProcess, &pMemCounters, sizeof(pMemCounters)))
        {
            m_pageFileUsage = static_cast<DWORD>(pMemCounters.PagefileUsage);
            m_virtualSize = static_cast<int32_t>(pMemCounters.WorkingSetSize + pMemCounters.PagefileUsage);
        }

        // else: Unable to retrieve page file usage from current process
    }

    /// @brief Returns the logical drives map
    /// @return Logical drives map
    static SystemDrivesMap getNtWin32DrivesMap()
    {
        SystemDrivesMap ret;

        // Get the total amount of available logical drives
        // The input size must not include the NULL terminator
        auto spLogicalDrives {std::make_unique<char[]>(OS_MAXSTR)};
        auto res {GetLogicalDriveStrings(OS_MAXSTR - 1, spLogicalDrives.get())};

        if (res <= 0 || res > OS_MAXSTR)
        {
            throw std::system_error {
                static_cast<int>(GetLastError()), std::system_category(), "Unable to parse logical drive strings."};
        }

        const auto logicalDrives {Utils::splitNullTerminatedStrings(spLogicalDrives.get())};

        for (const auto& logicalDrive : logicalDrives)
        {
            const auto normalizedName {logicalDrive.back() == L'\\' ? logicalDrive.substr(0, logicalDrive.length() - 1)
                                                                    : logicalDrive};
            const auto spDosDevice {std::make_unique<char[]>(OS_MAXSTR)};
            res = QueryDosDevice(normalizedName.c_str(), spDosDevice.get(), OS_MAXSTR);

            if (res)
            {
                // Make the NT Path <-> DOS Path mapping
                ret[spDosDevice.get()] = logicalDrive;
            }
        }

        // At least one logical drive couldn't be mapped, trying another use of QueryDosDevice
        if (logicalDrives.size() != ret.size())
        {
            // We avoid using OS_MAXSTR on Windows XP
            const auto spPhysicalDevices {std::make_unique<char[]>(OS_MAXSTR_HALF)};

            if (QueryDosDevice(nullptr, spPhysicalDevices.get(), OS_MAXSTR_HALF))
            {
                const auto tokens = Utils::splitNullTerminatedStrings(spPhysicalDevices.get());
                const auto spDosDevice {std::make_unique<char[]>(OS_MAXSTR_HALF)};

                for (const auto& token : tokens)
                {

                    // Checking if token is found in logicalDrives to avoid a large map with unnecessary volumes.
                    // logicalDrives contains a slash at the end but the result of QueryDosDevice doesn't.
                    auto startsWithLambda = [&](const auto& logicalDrive)
                    {
                        return Utils::startsWith(logicalDrive, token);
                    };

                    if (std::find_if(logicalDrives.begin(), logicalDrives.end(), startsWithLambda) ==
                        logicalDrives.end())
                    {
                        continue;
                    }

                    res = QueryDosDevice(token.c_str(), spDosDevice.get(), OS_MAXSTR_HALF);

                    if (res && ret.find(spDosDevice.get()) == ret.end())
                    {
                        ret[spDosDevice.get()] = token + '\\';

                        if (logicalDrives.size() == ret.size())
                        {
                            break;
                        }
                    }
                }
            }
        }

        return ret;
    }

    /// @brief Fills the output buffer with the DOS path
    /// @param drivesMap Logical drives map
    /// @param ntPath NT Path
    /// @param outbuf Output buffer
    /// @return True if the output buffer was filled
    bool fillOutput(const SystemDrivesMap& drivesMap, const std::string& ntPath, std::string& outbuf)
    {
        const auto it {std::find_if(drivesMap.begin(),
                                    drivesMap.end(),
                                    [&ntPath](const auto& key) -> bool
                                    { return Utils::startsWith(ntPath, key.first); })};

        const bool ret {it != drivesMap.end()};

        if (ret)
        {
            outbuf = it->second + ntPath.substr(it->first.size() + 1);
        }

        return ret;
    }

    /// @brief Converts NT Path to Win32 Path
    /// @param ntPath NT Path
    /// @param outbuf Output buffer
    void ntPath2Win32Path(const std::string& ntPath, std::string& outbuf)
    {
        static SystemDrivesMap s_drivesMap {getNtWin32DrivesMap()};

        if (!fillOutput(s_drivesMap, ntPath, outbuf))
        {
            s_drivesMap = getNtWin32DrivesMap();

            if (!fillOutput(s_drivesMap, ntPath, outbuf))
            {
                // If after re-fill the drives map DOS drive is not found, NTPath path will
                // be returned.
                outbuf = ntPath;
            }
        }
    }

    const DWORD m_pId;
    HANDLE m_hProcess;
    ULARGE_INTEGER m_creationTime;
    ULARGE_INTEGER m_kernelModeTime;
    ULARGE_INTEGER m_userModeTime;
    DWORD m_pageFileUsage;
    DWORD m_virtualSize;
};

static bool isSystemProcess(const DWORD pid)
{
    return pid == 0 || pid == 4;
}

static std::string processName(const PROCESSENTRY32& processEntry)
{
    std::string ret;
    const DWORD pId {processEntry.th32ProcessID};

    if (isSystemProcess(pId))
    {
        ret = (pId == 0) ? SYSTEM_IDLE_PROCESS_NAME : SYSTEM_PROCESS_NAME;
    }
    else
    {
        ret = processEntry.szExeFile;
    }

    return ret;
}

static nlohmann::json GetProcessInfo(const PROCESSENTRY32& processEntry)
{
    nlohmann::json jsProcessInfo {};
    const auto pId {processEntry.th32ProcessID};
    const auto processHandle {OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pId)};

    if (processHandle)
    {
        SysInfoProcess process(pId, processHandle);

        // Current process information
        jsProcessInfo["name"] = Utils::stringAnsiToStringUTF8(processName(processEntry));
        jsProcessInfo["cmd"] = Utils::stringAnsiToStringUTF8((isSystemProcess(pId)) ? "none" : process.cmd());
        jsProcessInfo["stime"] = process.kernelModeTime();
        jsProcessInfo["size"] = process.pageFileUsage();
        jsProcessInfo["ppid"] = processEntry.th32ParentProcessID;
        jsProcessInfo["priority"] = processEntry.pcPriClassBase;
        jsProcessInfo["pid"] = std::to_string(pId);
        jsProcessInfo["session"] = process.sessionId();
        jsProcessInfo["nlwp"] = processEntry.cntThreads;
        jsProcessInfo["utime"] = process.userModeTime();
        jsProcessInfo["vm_size"] = process.virtualSize();
        jsProcessInfo["start_time"] = process.creationTime();
        CloseHandle(processHandle);
    }

    return jsProcessInfo;
}

static void getPackagesFromReg(const HKEY key,
                               const std::string& subKey,
                               std::function<void(nlohmann::json&)> returnCallback,
                               const REGSAM access = 0)
{
    try
    {
        const auto callback {
            [&](const std::string& package)
            {
                std::string value;
                nlohmann::json packageJson;
                Utils::Registry packageReg {key, subKey + "\\" + package, access | KEY_READ};

                if (packageReg.string("DisplayName", value) && !value.empty())
                {
                    packageJson["name"] = value;

                    packageJson["version"] = EMPTY_VALUE;
                    if (packageReg.string("DisplayVersion", value))
                    {
                        packageJson["version"] = value;
                    }

                    packageJson["vendor"] = UNKNOWN_VALUE;
                    if (packageReg.string("Publisher", value))
                    {
                        packageJson["vendor"] = value;
                    }

                    if (packageReg.string("InstallDate", value))
                    {
                        try
                        {
                            packageJson["install_time"] = Utils::timestampToISO8601(
                                Utils::normalizeTimestamp(value, packageReg.keyModificationDate()));
                        }
                        catch (const std::exception& e)
                        {
                            (void)e;
                            packageJson["install_time"] = Utils::timestampToISO8601(packageReg.keyModificationDate());
                        }
                    }
                    else
                    {
                        packageJson["install_time"] = Utils::timestampToISO8601(packageReg.keyModificationDate());
                    }

                    packageJson["location"] = EMPTY_VALUE;
                    if (packageReg.string("InstallLocation", value))
                    {
                        packageJson["location"] = value;
                    }

                    packageJson["architecture"] = EMPTY_VALUE;
                    if (access & KEY_WOW64_32KEY)
                    {
                        packageJson["architecture"] = "i686";
                    }
                    else if (access & KEY_WOW64_64KEY)
                    {
                        packageJson["architecture"] = "x86_64";
                    }

                    packageJson["description"] = UNKNOWN_VALUE;
                    packageJson["groups"] = UNKNOWN_VALUE;
                    packageJson["priority"] = UNKNOWN_VALUE;
                    packageJson["size"] = UNKNOWN_VALUE;
                    packageJson["source"] = UNKNOWN_VALUE;
                    packageJson["format"] = "win";

                    returnCallback(packageJson);
                }
            }};
        Utils::Registry root {key, subKey, access | KEY_ENUMERATE_SUB_KEYS | KEY_READ};
        root.enumerate(callback);
    }
    catch (...) // NOLINT(bugprone-empty-catch)
    {
    }
}

static void
getStorePackages(const HKEY key, const std::string& user, std::function<void(nlohmann::json&)> returnCallback)
{
    std::set<std::string> cacheReg;

    try
    {
        for (const auto& registry :
             Utils::Registry {key, user + "\\" + CACHE_NAME_REGISTRY, KEY_READ | KEY_ENUMERATE_SUB_KEYS}.enumerate())
        {
            cacheReg.insert(registry);
        }

        const auto callback {
            [&](const std::string& nameApp)
            {
                nlohmann::json jsPackage;

                FactoryWindowsPackage::create(key, user, nameApp, cacheReg)->buildPackageData(jsPackage);

                if (!jsPackage.at("name").get_ref<const std::string&>().empty())
                {
                    // Only return valid content packages
                    returnCallback(jsPackage);
                }
            }};

        Utils::Registry root(key, user + "\\" + APPLICATION_STORE_REGISTRY, KEY_READ | KEY_ENUMERATE_SUB_KEYS);
        root.enumerate(callback);
    }
    catch (...) // NOLINT(bugprone-empty-catch)
    {
    }
}

static std::string GetSerialNumber()
{
    std::string ret;

    if (Utils::isVistaOrLater())
    {
        static auto pfnGetSystemFirmwareTable {Utils::getSystemFirmwareTableFunctionAddress()};

        if (pfnGetSystemFirmwareTable)
        {
            const auto size {pfnGetSystemFirmwareTable(gs_firmwareTableProviderSignature.at("RSMB"), 0, nullptr, 0)};

            if (size)
            {
                const auto spBuff {std::make_unique<unsigned char[]>(size)};

                if (spBuff)
                {
                    // Get raw SMBIOS firmware table
                    if (pfnGetSystemFirmwareTable(
                            gs_firmwareTableProviderSignature.at("RSMB"), 0, spBuff.get(), size) == size)
                    {
                        PRawSMBIOSData smbios {reinterpret_cast<PRawSMBIOSData>(spBuff.get())};
                        // Parse SMBIOS structures
                        ret = Utils::getSerialNumberFromSmbios(smbios->SMBIOSTableData, smbios->Length);
                    }
                }
            }
        }
    }
    else
    {
        const auto rawData {Utils::Exec("wmic baseboard get SerialNumber")};
        const auto pos {rawData.find("\r\n")};

        if (pos != std::string::npos)
        {
            ret = Utils::Trim(rawData.substr(pos), " \t\r\n");
        }
        else
        {
            ret = EMPTY_VALUE;
        }
    }

    return ret;
}

static std::string GetCpuName()
{
    Utils::Registry reg(HKEY_LOCAL_MACHINE, CENTRAL_PROCESSOR_REGISTRY);
    return reg.string("ProcessorNameString");
}

static int GetCpuMHz()
{
    Utils::Registry reg(HKEY_LOCAL_MACHINE, CENTRAL_PROCESSOR_REGISTRY);
    return reg.dword("~MHz");
}

static int GetCpuCores()
{
    SYSTEM_INFO siSysInfo {};
    GetSystemInfo(&siSysInfo);
    return siSysInfo.dwNumberOfProcessors;
}

static void GetMemory(nlohmann::json& info)
{
    MEMORYSTATUSEX statex;
    statex.dwLength = sizeof(statex);

    if (GlobalMemoryStatusEx(&statex))
    {
        info["ram_total"] = statex.ullTotalPhys / KByte;
        info["ram_free"] = statex.ullAvailPhys / KByte;
        info["ram_usage"] = statex.dwMemoryLoad;
    }
    else
    {
        throw std::system_error {
            static_cast<int>(GetLastError()), std::system_category(), "Error calling GlobalMemoryStatusEx"};
    }
}

nlohmann::json SysInfo::getHardware() const
{
    nlohmann::json hardware;
    hardware["board_serial"] = GetSerialNumber();
    hardware["cpu_name"] = GetCpuName();
    hardware["cpu_cores"] = GetCpuCores();
    hardware["cpu_mhz"] = GetCpuMHz();
    GetMemory(hardware);
    return hardware;
}

static void fillProcessesData(std::function<void(PROCESSENTRY32)> func)
{
    PROCESSENTRY32 processEntry {};
    processEntry.dwSize = sizeof(PROCESSENTRY32);
    const auto processesSnapshot {CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0)};

    if (INVALID_HANDLE_VALUE != processesSnapshot)
    {
        if (Process32First(processesSnapshot, &processEntry))
        {
            do
            {
                func(processEntry);
            } while (Process32Next(processesSnapshot, &processEntry));
        }
        else
        {
            CloseHandle(processesSnapshot);
            throw std::system_error {static_cast<int>(GetLastError()),
                                     std::system_category(),
                                     "Unable to retrieve process information from the snapshot."};
        }

        CloseHandle(processesSnapshot);
    }
    else
    {
        throw std::system_error {
            static_cast<int>(GetLastError()), std::system_category(), "Unable to create process snapshot."};
    }
}

nlohmann::json SysInfo::getProcessesInfo() const
{
    nlohmann::json jsProcessesList {};

    getProcessesInfo([&jsProcessesList](nlohmann::json& data) { jsProcessesList.push_back(data); });

    return jsProcessesList;
}

nlohmann::json SysInfo::getPackages() const
{
    nlohmann::json ret;
    getPackages([&ret](nlohmann::json& data) { ret.push_back(data); });
    return ret;
}

nlohmann::json SysInfo::getOsInfo() const
{
    nlohmann::json ret;
    const auto spOsInfoProvider {std::make_shared<SysOsInfoProviderWindows>()};
    SysOsInfo::setOsInfo(spOsInfoProvider, ret);
    return ret;
}

nlohmann::json SysInfo::getNetworks() const
{
    nlohmann::json networks {};
    std::unique_ptr<IP_ADAPTER_INFO, Utils::IPAddressSmartDeleter> adapterInfo;
    std::unique_ptr<IP_ADAPTER_ADDRESSES, Utils::IPAddressSmartDeleter> adaptersAddresses;
    Utils::getAdapters(adaptersAddresses);

    if (!Utils::isVistaOrLater())
    {
        // Get additional IPv4 info - Windows XP only
        Utils::getAdapterInfo(adapterInfo);
    }

    auto rawAdapterAddresses {adaptersAddresses.get()};

    while (rawAdapterAddresses)
    {
        nlohmann::json netInterfaceInfo {};

        if ((IF_TYPE_SOFTWARE_LOOPBACK != rawAdapterAddresses->IfType) ||
            (0 != rawAdapterAddresses->IfIndex || 0 != rawAdapterAddresses->Ipv6IfIndex))
        {
            // Ignore either loopback and invalid IPv4/IPv6 indexes interfaces

            auto unicastAddress {rawAdapterAddresses->FirstUnicastAddress};

            while (unicastAddress)
            {
                const auto lpSockAddr {unicastAddress->Address.lpSockaddr};

                if (lpSockAddr)
                {
                    const auto unicastAddressFamily {lpSockAddr->sa_family};

                    if (AF_INET == unicastAddressFamily)
                    {
                        // IPv4 data
                        FactoryNetworkFamilyCreator<OSPlatformType::WINDOWS>::create(
                            std::make_shared<NetworkWindowsInterface>(
                                Utils::IPV4, rawAdapterAddresses, unicastAddress, adapterInfo.get()))
                            ->buildNetworkData(netInterfaceInfo);
                    }
                    else if (AF_INET6 == unicastAddressFamily)
                    {
                        // IPv6 data
                        FactoryNetworkFamilyCreator<OSPlatformType::WINDOWS>::create(
                            std::make_shared<NetworkWindowsInterface>(
                                Utils::IPV6, rawAdapterAddresses, unicastAddress, adapterInfo.get()))
                            ->buildNetworkData(netInterfaceInfo);
                    }
                }

                unicastAddress = unicastAddress->Next;
            }

            // Common data
            FactoryNetworkFamilyCreator<OSPlatformType::WINDOWS>::create(
                std::make_shared<NetworkWindowsInterface>(
                    Utils::COMMON_DATA, rawAdapterAddresses, unicastAddress, adapterInfo.get()))
                ->buildNetworkData(netInterfaceInfo);

            networks["iface"].push_back(netInterfaceInfo);
        }

        rawAdapterAddresses = rawAdapterAddresses->Next;
    }

    return networks;
}

template<class T, typename TableClass>
void getTablePorts(TableClass ownerId,
                   int32_t tcpipVersion,
                   std::unique_ptr<T[]>& tableList,
                   std::function<DWORD(T*, DWORD*, bool, int32_t, TableClass)> GetTable)
{
    TableClass classId {ownerId};
    DWORD size {0};

    if (ERROR_INSUFFICIENT_BUFFER == GetTable(nullptr, &size, true, tcpipVersion, classId))
    {
        tableList = std::make_unique<T[]>(size);

        if (tableList)
        {
            if (NO_ERROR != GetTable(tableList.get(), &size, true, tcpipVersion, classId))
            {
                throw std::runtime_error("Error when get table information");
            }
        }
    }
}

template<typename T>
void expandPortData(T data, const std::map<pid_t, std::string>& processDataList, nlohmann::json& result)
{
    if (data)
    {
        for (auto i = 0ul; i < data->dwNumEntries; ++i)
        {
            nlohmann::json port;
            std::make_unique<PortImpl>(std::make_shared<WindowsPortWrapper>(data->table[i], processDataList))
                ->buildPortData(port);
            result.push_back(port);
        }
    }
}

nlohmann::json SysInfo::getPorts() const
{
    nlohmann::json ports;
    std::map<pid_t, std::string> processDataList;
    PortTables portTable;

    fillProcessesData([&processDataList](const auto& processEntry)
                      { processDataList[processEntry.th32ProcessID] = processEntry.szExeFile; });

    getTablePorts<MIB_TCPTABLE_OWNER_PID, TCP_TABLE_CLASS>(
        TCP_TABLE_OWNER_PID_ALL,
        AF_INET,
        portTable.tcp,
        [](MIB_TCPTABLE_OWNER_PID* table, DWORD* size, bool order, int32_t tcpipVersion, TCP_TABLE_CLASS tableClass)
        { return GetExtendedTcpTable(table, size, order, tcpipVersion, tableClass, 0); });
    expandPortData(portTable.tcp.get(), processDataList, ports);

    getTablePorts<MIB_TCP6TABLE_OWNER_PID, TCP_TABLE_CLASS>(
        TCP_TABLE_OWNER_PID_ALL,
        AF_INET6,
        portTable.tcp6,
        [](MIB_TCP6TABLE_OWNER_PID* table, DWORD* size, bool order, int32_t tcpipVersion, TCP_TABLE_CLASS tableClass)
        { return GetExtendedTcpTable(table, size, order, tcpipVersion, tableClass, 0); });
    expandPortData(portTable.tcp6.get(), processDataList, ports);

    getTablePorts<MIB_UDPTABLE_OWNER_PID, UDP_TABLE_CLASS>(
        UDP_TABLE_OWNER_PID,
        AF_INET,
        portTable.udp,
        [](MIB_UDPTABLE_OWNER_PID* table, DWORD* size, bool order, int32_t tcpipVersion, UDP_TABLE_CLASS tableClass)
        { return GetExtendedUdpTable(table, size, order, tcpipVersion, tableClass, 0); });
    expandPortData(portTable.udp.get(), processDataList, ports);

    getTablePorts<MIB_UDP6TABLE_OWNER_PID, UDP_TABLE_CLASS>(
        UDP_TABLE_OWNER_PID,
        AF_INET6,
        portTable.udp6,
        [](MIB_UDP6TABLE_OWNER_PID* table, DWORD* size, bool order, int32_t tcpipVersion, UDP_TABLE_CLASS tableClass)
        { return GetExtendedUdpTable(table, size, order, tcpipVersion, tableClass, 0); });
    expandPortData(portTable.udp6.get(), processDataList, ports);

    return ports;
}

void SysInfo::getProcessesInfo(const std::function<void(nlohmann::json&)>& callback) const
{
    fillProcessesData(
        [&callback](const auto& processEntry)
        {
            auto processInfo = GetProcessInfo(processEntry);

            if (!processInfo.empty())
            {
                callback(processInfo);
            }
        });
}

void expandFromRegistry(const HKEY key,
                        const std::string& subKey,
                        const std::string& field,
                        std::function<void(const std::string&)> postAction)
{
    std::vector<std::string> installPaths;

    // Get install path from registry
    Utils::expandRegistryPath(key, subKey, installPaths);

    // Get install path from registry expanded keys.
    for (const auto& versionKey : installPaths)
    {
        try
        {
            // Get install path from registry, based on version key.
            const auto dir {Utils::Registry {key, versionKey, KEY_READ | KEY_WOW64_64KEY}.string(field)};
            // Add install path to dirList.
            postAction(dir);
        }
        catch (const std::exception& e)
        {
            // Ignore errors.
            (void)e;
        }
    }
}

const std::set<std::string> getPythonDirectories()
{
    std::set<std::string> pythonDirList;

    const auto postAction = [&](const std::string& pythonDir)
    {
        pythonDirList.insert(pythonDir + R"(Lib\site-packages)");
        pythonDirList.insert(pythonDir + R"(Lib\dist-packages)");
    };

    try
    {
        expandFromRegistry(HKEY_USERS, R"(*\SOFTWARE\Python\PythonCore\*\InstallPath)", "", postAction);
        expandFromRegistry(HKEY_LOCAL_MACHINE, R"(SOFTWARE\Python\PythonCore\*\InstallPath)", "", postAction);
    }
    catch (const std::exception&)
    {
        // Ignore errors.
    }

    return pythonDirList;
}

const std::set<std::string> getNodeDirectories()
{
    std::set<std::string> nodeDirList;

    const auto postAction = [&](const std::string& nodeDir)
    {
        nodeDirList.insert(nodeDir);
    };

    try
    {
        expandFromRegistry(HKEY_USERS, R"(*\SOFTWARE\Node.js)", "InstallPath", postAction);
        expandFromRegistry(HKEY_LOCAL_MACHINE, R"(SOFTWARE\Node.js)", "InstallPath", postAction);
        nodeDirList.insert(R"(C:\Users\*\AppData\Roaming\npm)");
        nodeDirList.insert(R"(C:\Users\*)");
    }
    catch (const std::exception&)
    {
        // Ignore errors.
    }

    return nodeDirList;
}

void SysInfo::getPackages(const std::function<void(nlohmann::json&)>& callback) const
{
    std::set<std::string> set;

    auto fillList {[&callback, &set](nlohmann::json& data)
                   {
                       const std::string key {data.at("name").get_ref<const std::string&>() +
                                              data.at("version").get_ref<const std::string&>()};
                       const auto result {set.insert(key)};

                       if (result.second)
                       {
                           callback(data);
                       }
                   }};

    getPackagesFromReg(HKEY_LOCAL_MACHINE, UNINSTALL_REGISTRY, fillList, KEY_WOW64_64KEY);
    getPackagesFromReg(HKEY_LOCAL_MACHINE, UNINSTALL_REGISTRY, fillList, KEY_WOW64_32KEY);

    for (const auto& user : Utils::Registry {HKEY_USERS, "", KEY_READ | KEY_ENUMERATE_SUB_KEYS}.enumerate())
    {
        getPackagesFromReg(HKEY_USERS, user + "\\" + UNINSTALL_REGISTRY, fillList);
        getStorePackages(HKEY_USERS, user, fillList);
    }

    const std::map<std::string, std::set<std::string>> searchPaths = {{"PYPI", getPythonDirectories()},
                                                                      {"NPM", getNodeDirectories()}};

    ModernFactoryPackagesCreator::getPackages(searchPaths, callback);
}

nlohmann::json SysInfo::getHotfixes() const
{
    std::set<std::string> hotfixes;
    PackageWindowsHelper::getHotFixFromReg(HKEY_LOCAL_MACHINE, PackageWindowsHelper::WIN_REG_HOTFIX, hotfixes);
    PackageWindowsHelper::getHotFixFromRegNT(HKEY_LOCAL_MACHINE, PackageWindowsHelper::VISTA_REG_HOTFIX, hotfixes);
    PackageWindowsHelper::getHotFixFromRegWOW(HKEY_LOCAL_MACHINE, PackageWindowsHelper::WIN_REG_WOW_HOTFIX, hotfixes);
    PackageWindowsHelper::getHotFixFromRegProduct(
        HKEY_LOCAL_MACHINE, PackageWindowsHelper::WIN_REG_PRODUCT_HOTFIX, hotfixes);

    nlohmann::json ret;

    for (auto& hotfix : hotfixes)
    {
        nlohmann::json hotfixValue;
        hotfixValue["hotfix"] = std::move(hotfix);
        ret.push_back(std::move(hotfixValue));
    }

    return ret;
}
