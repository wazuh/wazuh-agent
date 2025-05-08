#include "networkWindowsHelper.hpp"

#include "encodingWindowsHelper.hpp"
#include "stringHelper.hpp"
#include "windowsHelper.hpp"
#include <WTypesbase.h>
#include <array>
#include <iomanip>
#include <map>
#include <memory>
#include <sstream>
#include <string>
#include <system_error>

constexpr auto WORKING_ADAPTERS_INFO_BUFFER_SIZE {15000};
constexpr auto MAX_ADAPTERS_INFO_TRIES {3};

namespace Utils
{
    GetSystemFirmwareTable_t getSystemFirmwareTableFunctionAddress()
    {
        GetSystemFirmwareTable_t ret {nullptr};
        auto hKernel32 {GetModuleHandle(TEXT("kernel32.dll"))};

        if (hKernel32)
        {
            ret = reinterpret_cast<GetSystemFirmwareTable_t>(GetProcAddress(hKernel32, "GetSystemFirmwareTable"));
        }

        return ret;
    }

    ConvertLengthToIpv4Mask_t getConvertLengthToIpv4MaskFunctionAddress()
    {
        ConvertLengthToIpv4Mask_t ret {nullptr};
        auto hIphlpapi {GetModuleHandle(TEXT("Iphlpapi.dll"))};

        if (hIphlpapi)
        {
            ret = reinterpret_cast<ConvertLengthToIpv4Mask_t>(GetProcAddress(hIphlpapi, "ConvertLengthToIpv4Mask"));
        }

        return ret;
    }

    GetIfEntry2_t getIfEntry2FunctionAddress()
    {
        GetIfEntry2_t ret {nullptr};
        auto hIphlpapi {GetModuleHandle(TEXT("Iphlpapi.dll"))};

        if (hIphlpapi)
        {
            ret = reinterpret_cast<GetIfEntry2_t>(GetProcAddress(hIphlpapi, "GetIfEntry2"));
        }

        return ret;
    }

    inet_pton_t getInetPtonFunctionAddress()
    {
        inet_pton_t ret {nullptr};
        auto hWs232 {GetModuleHandle(TEXT("ws2_32.dll"))};

        if (hWs232)
        {
            ret = reinterpret_cast<inet_pton_t>(GetProcAddress(hWs232, "inet_pton"));
        }

        return ret;
    }

    inet_ntop_t getInetNtopFunctionAddress()
    {
        inet_ntop_t ret {nullptr};
        auto hWs232 {GetModuleHandle(TEXT("ws2_32.dll"))};

        if (hWs232)
        {
            ret = reinterpret_cast<inet_ntop_t>(GetProcAddress(hWs232, "inet_ntop"));
        }

        return ret;
    }

    DWORD getAdapterAddresses(PIP_ADAPTER_ADDRESSES& ipAdapterAddresses)
    {
        // Set the flags to pass to GetAdaptersAddresses()
        // When the GAA_FLAG_INCLUDE_PREFIX flag is set, IP address prefixes are returned for both IPv6 and IPv4
        // addresses.
        const auto adapterAddressesFlags {
            Utils::isVistaOrLater() ? (GAA_FLAG_INCLUDE_PREFIX | GAA_FLAG_INCLUDE_GATEWAYS) : 0};

        ULONG bufferLen {WORKING_ADAPTERS_INFO_BUFFER_SIZE};
        DWORD dwRetVal {0};
        auto attempts {0};
        bool adapterAddressesFound {false};

        do
        {
            ipAdapterAddresses = reinterpret_cast<IP_ADAPTER_ADDRESSES*>(win_alloc(bufferLen));

            if (!ipAdapterAddresses)
            {
                throw std::system_error {static_cast<int>(GetLastError()),
                                         std::system_category(),
                                         "Unable to allocate memory for PIP_ADAPTER_ADDRESSES struct."};
            }

            // Two calls of GetAdaptersAddresses are needed. One for getting the size needed (bufferLen variable),
            // and the second one for getting the actual data we want.
            dwRetVal = GetAdaptersAddresses(AF_UNSPEC, adapterAddressesFlags, nullptr, ipAdapterAddresses, &bufferLen);

            if (ERROR_BUFFER_OVERFLOW == dwRetVal)
            {
                win_free(ipAdapterAddresses);
                ipAdapterAddresses = nullptr;
            }
            else
            {
                adapterAddressesFound = true;
            }

            ++attempts;
        } while (!adapterAddressesFound && attempts < MAX_ADAPTERS_INFO_TRIES);

        return dwRetVal;
    }

    DWORD getAdapterInfoXP(PIP_ADAPTER_INFO& ipAdapterInfo)
    {
        // Windows XP additional IPv4 interfaces data

        ULONG bufferLen {WORKING_ADAPTERS_INFO_BUFFER_SIZE};
        DWORD dwRetVal {0};
        auto attempts {0};
        bool adapterInfoFound {false};

        while (!adapterInfoFound && attempts < MAX_ADAPTERS_INFO_TRIES)
        {
            ipAdapterInfo = reinterpret_cast<IP_ADAPTER_INFO*>(win_alloc(bufferLen));

            if (!ipAdapterInfo)
            {
                throw std::system_error {static_cast<int>(GetLastError()),
                                         std::system_category(),
                                         "Unable to allocate memory for IP_ADAPTER_INFO struct."};
            }

            dwRetVal = GetAdaptersInfo(ipAdapterInfo, &bufferLen);

            if (ERROR_BUFFER_OVERFLOW == dwRetVal)
            {
                win_free(ipAdapterInfo);
                ipAdapterInfo = nullptr;
            }
            else
            {
                adapterInfoFound = true;
            }

            ++attempts;
        }

        return dwRetVal;
    }

    std::string getAdapterNameStr(const std::wstring& adapterName)
    {
        return Utils::wstringToStringUTF8(adapterName);
    }

    void getAdapters(std::unique_ptr<IP_ADAPTER_ADDRESSES, IPAddressSmartDeleter>& interfacesAddress)
    {
        PIP_ADAPTER_ADDRESSES ipAdapterAddresses {nullptr};
        const DWORD dwRetVal {getAdapterAddresses(ipAdapterAddresses)};

        if (NO_ERROR == dwRetVal)
        {
            interfacesAddress.reset(ipAdapterAddresses);
        }
        else
        {
            throw std::system_error {
                static_cast<int>(dwRetVal), std::system_category(), "Error reading network adapter addresses"};
        }
    }

    void getAdapterInfo(std::unique_ptr<IP_ADAPTER_INFO, IPAddressSmartDeleter>& adapterInfo)
    {
        PIP_ADAPTER_INFO ipAdapterInfo {nullptr};
        const DWORD dwRetVal {getAdapterInfoXP(ipAdapterInfo)};

        if (NO_ERROR == dwRetVal)
        {
            adapterInfo.reset(ipAdapterInfo);
        }
        else
        {
            throw std::system_error {
                static_cast<int>(dwRetVal), std::system_category(), "Error reading network adapter info"};
        }
    }

    std::string IAddressToString(const int family, in_addr address)
    {
        std::string retVal;
        auto plainAddress {std::make_unique<char[]>(NI_MAXHOST)};

        if (Utils::isVistaOrLater())
        {
            auto pfnInetNtop {getInetNtopFunctionAddress()};

            if (pfnInetNtop)
            {
                if (pfnInetNtop(family, &address, plainAddress.get(), NI_MAXHOST))
                {
                    retVal = plainAddress.get();
                }
            }
        }
        else
        {
#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
            // Windows XP
            plainAddress.reset(inet_ntoa(address));
#ifdef _MSC_VER
#pragma warning(pop)
#endif

            if (plainAddress)
            {
                retVal = plainAddress.get();
            }
        }

        return retVal;
    }

    std::string IAddressToString(const int family, in6_addr address)
    {
        std::string retVal;
        auto plainAddress {std::make_unique<char[]>(NI_MAXHOST)};

        if (Utils::isVistaOrLater())
        {
            auto pfnInetNtop {getInetNtopFunctionAddress()};

            if (pfnInetNtop)
            {
                if (pfnInetNtop(family, &address, plainAddress.get(), NI_MAXHOST))
                {
                    retVal = plainAddress.get();
                }
            }
        }

        // IPv6 in Windows XP is not supported
        return retVal;
    }

    std::string broadcastAddress(const std::string& ipAddress, const std::string& netmask)
    {
        struct in_addr host
        {
        };

        struct in_addr mask
        {
        };

        struct in_addr broadcast
        {
        };

        std::string broadcastAddr;

        auto pfnInetPton {getInetPtonFunctionAddress()};

        if (pfnInetPton)
        {
            if (pfnInetPton(AF_INET, ipAddress.c_str(), &host) == 1 &&
                pfnInetPton(AF_INET, netmask.c_str(), &mask) == 1)
            {
                broadcast.s_addr = host.s_addr | ~mask.s_addr;
                broadcastAddr = IAddressToString(AF_INET, broadcast);
            }
        }

        return broadcastAddr;
    }

    std::string getIpV6Address(const uint8_t* addrParam)
    {
        if (!addrParam)
        {
            return "";
        }

        sockaddr_in6 sa6 {};
        sa6.sin6_family = AF_INET6;
        memcpy(&sa6.sin6_addr, addrParam, 16);

        wchar_t buffer[128] = {};
        DWORD bufSize = 128;

        if (WSAAddressToStringW(reinterpret_cast<SOCKADDR*>(&sa6), sizeof(sa6), nullptr, buffer, &bufSize) != 0)
        {
            return "";
        }

        return wstringToStringUTF8(buffer);
    }

    std::string ipv6Netmask(const uint8_t maskLength)
    {
        const auto MAX_BITS_LENGTH {128};
        std::string netmask;

        if (maskLength < MAX_BITS_LENGTH)
        {
            // For a unicast IPv6 address, any value greater than 128 is an illegal value

            // Each chunks of addresses has four letters "f" following by a ":"
            // If "maskLength" is not multiple of 4, we need to fill the current
            // "chunk" depending of the amount of letters needed. That's why
            // the need of the following map.
            std::map<int, std::string> NET_MASK_FILLER_CHARS_MAP = {{1, "8"}, {2, "c"}, {3, "e"}};
            const int BITS_PER_CHUNK {4};
            const int NETMASK_TOTAL_BITS {32};

            netmask = "ffff:ffff:ffff:ffff:ffff:ffff:ffff:ffff"; // 128 bits address
            const int value {maskLength / BITS_PER_CHUNK};
            const int remainingValue {maskLength % BITS_PER_CHUNK};
            const int totalSum {value + remainingValue};
            const int refillData {totalSum % BITS_PER_CHUNK};
            const int separators {value / BITS_PER_CHUNK};
            const int remainingSeparators {value % BITS_PER_CHUNK};
            const int finalNumberOfSeparators {remainingSeparators == 0 ? separators - 1 : separators};

            // Add the needed ":" separators
            netmask = netmask.substr(0, value + finalNumberOfSeparators);

            if (remainingValue)
            {
                // If the maskLength is not multiple of 4, let's refill with the corresponding
                // character
                const auto it {NET_MASK_FILLER_CHARS_MAP.find(remainingValue)};

                if (NET_MASK_FILLER_CHARS_MAP.end() != it)
                {
                    netmask += it->second;
                }
            }
            else
            {
                netmask += std::string(refillData, '0'); // Refill data with 0's if applies
            }

            if (totalSum < (NETMASK_TOTAL_BITS - BITS_PER_CHUNK))
            {
                // Append "::" to fill the complete 128 bits address (IPv6 representation)
                netmask += "::";
            }
        }

        return netmask;
    }
} // namespace Utils
