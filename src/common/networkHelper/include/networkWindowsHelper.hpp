#pragma once

// clang-format off
#include <memory>
#include <string>
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <netioapi.h>
// clang-format on

#define win_alloc(x) HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, (x))
#define win_free(x)  HeapFree(GetProcessHeap(), 0, (x))

namespace Utils
{
    typedef unsigned int UINT;
    typedef ULONG(WINAPI* ConvertLengthToIpv4Mask_t)(ULONG, PULONG);
    typedef ULONG(WINAPI* GetIfEntry2_t)(PMIB_IF_ROW2);
    typedef UINT(WINAPI* GetSystemFirmwareTable_t)(DWORD, DWORD, PVOID, DWORD);
    typedef INT(WINAPI* inet_pton_t)(INT, PCSTR, PVOID);
    typedef PCSTR(WINAPI* inet_ntop_t)(INT, PVOID, PSTR, size_t);

    /// @brief Smart deleter for IP_ADAPTER_INFO and IP_ADAPTER_ADDRESSES
    struct IPAddressSmartDeleter
    {
        void operator()(IP_ADAPTER_INFO* address)
        {
            win_free(address);
            address = nullptr;
        }

        void operator()(IP_ADAPTER_ADDRESSES* address)
        {
            win_free(address);
            address = nullptr;
        }
    };

    /// @brief Network family types
    enum NetworkFamilyTypes
    {
        UNDEF,
        IPV4,
        IPV6,
        COMMON_DATA
    };

    /// @brief Get the system firmware table function address
    /// @return the system firmware table function address
    GetSystemFirmwareTable_t getSystemFirmwareTableFunctionAddress();

    /// @brief Get the convert length to ipv4 mask function address
    /// @return the convert length to ipv4 mask function address
    ConvertLengthToIpv4Mask_t getConvertLengthToIpv4MaskFunctionAddress();

    /// @brief Get the get if entry 2 function address
    /// @return the get if entry 2 function address
    GetIfEntry2_t getIfEntry2FunctionAddress();

    /// @brief Get the inet pton function address
    /// @return the inet pton function address
    inet_pton_t getInetPtonFunctionAddress();

    /// @brief Get the inet ntop function address
    /// @return the inet ntop function address
    inet_ntop_t getInetNtopFunctionAddress();

    /// @brief Get the adapter addresses
    /// @param ipAdapterAddresses the adapter addresses
    /// @return the error code
    DWORD getAdapterAddresses(PIP_ADAPTER_ADDRESSES& ipAdapterAddresses);

    /// @brief Get the adapter info
    /// @param ipAdapterInfo the adapter info
    /// @return the error code
    DWORD getAdapterInfoXP(PIP_ADAPTER_INFO& ipAdapterInfo);

    /// @brief Get the adapter name as string
    /// @param adapterName the adapter name
    /// @return the adapter name
    std::string getAdapterNameStr(const std::wstring& adapterName);

    /// @brief Get the adapters
    /// @param interfacesAddress the interfaces address
    void getAdapters(std::unique_ptr<IP_ADAPTER_ADDRESSES, IPAddressSmartDeleter>& interfacesAddress);

    /// @brief Get the adapter info
    /// @param adapterInfo the adapter info
    void getAdapterInfo(std::unique_ptr<IP_ADAPTER_INFO, IPAddressSmartDeleter>& adapterInfo);

    /// @brief Convert the address to string for ipv4
    /// @param family the family
    /// @param address the address
    /// @return the string
    std::string IAddressToString(const int family, in_addr address);

    /// @brief Convert the address to string for ipv6
    /// @param family the family
    /// @param address the address
    /// @return the string
    std::string IAddressToString(const int family, in6_addr address);

    /// @brief Get the broadcast address
    /// @param ipAddress the ip address
    /// @param netmask the netmask
    /// @return the broadcast address
    std::string broadcastAddress(const std::string& ipAddress, const std::string& netmask);

    /// @brief Get the ip v6 address
    /// @param addrParam the address
    /// @return the ip v6 address
    std::string getIpV6Address(const uint8_t* addrParam);

    /// @brief Get the ip v6 netmask
    /// @param maskLength the mask length
    /// @return the ip v6 netmask
    std::string ipv6Netmask(const uint8_t maskLength);
} // namespace Utils
