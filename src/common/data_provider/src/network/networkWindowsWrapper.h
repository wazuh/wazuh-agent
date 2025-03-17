#pragma once

#include "inetworkWrapper.h"
#include "networkWindowsHelper.hpp"
#include "sharedDefs.h"
#include "windowsHelper.hpp"
#include <ifdef.h>
#include <iomanip>
#include <iptypes.h>
#include <netioapi.h>
#include <sstream>

static const std::map<int, std::string> NETWORK_INTERFACE_TYPES = {
    {IF_TYPE_ETHERNET_CSMACD, "ethernet"},
    {IF_TYPE_ISO88025_TOKENRING, "token ring"},
    {IF_TYPE_PPP, "point-to-point"},
    {IF_TYPE_ATM, "ATM"},
    {IF_TYPE_IEEE80211, "wireless"},
    {IF_TYPE_TUNNEL, "tunnel"},
    {IF_TYPE_IEEE1394, "firewire"},
};

static const std::map<IF_OPER_STATUS, std::string> NETWORK_OPERATIONAL_STATUS = {
    {IfOperStatusUp, "up"},
    {IfOperStatusDown, "down"},
    {IfOperStatusTesting, "testing"}, // In testing mode
    {IfOperStatusUnknown, "unknown"},
    {IfOperStatusDormant, "dormant"}, // In a pending state, waiting for some external event
    {IfOperStatusNotPresent,
     "notpresent"}, // Interface down because of any component is not present (hardware typically)
    {IfOperStatusLowerLayerDown, "lowerlayerdown"}, // This interface depends on a lower layer interface which is down
};

/// @brief Windows network interface
class NetworkWindowsInterface final : public INetworkInterfaceWrapper
{
public:
    /// @brief Constructor
    /// @param family interface family
    /// @param addrs interface address
    /// @param unicastAddress unicast address
    /// @param adapterInfo adapter info
    explicit NetworkWindowsInterface(Utils::NetworkFamilyTypes family,
                                     const PIP_ADAPTER_ADDRESSES& addrs,
                                     const PIP_ADAPTER_UNICAST_ADDRESS& unicastAddress,
                                     const PIP_ADAPTER_INFO& adapterInfo)
        : m_interfaceFamily(family)
        , m_interfaceAddress(addrs)
        , m_currentUnicastAddress(unicastAddress)
        , m_adapterInfo(adapterInfo)
    {
        if (!addrs)
        {
            throw std::runtime_error {"Nullptr instance of network interface"};
        }

        if (Utils::NetworkFamilyTypes::UNDEF == family)
        {
            throw std::runtime_error {"Undefined network instance family value"};
        }
    }

    /// @copydoc INetworkInterfaceWrapper::name
    std::string name() const override
    {
        return getAdapterEncodedUTF8(m_interfaceAddress->FriendlyName);
    }

    /// @copydoc INetworkInterfaceWrapper::adapter
    void adapter(nlohmann::json& network) const override
    {
        network["adapter"] = getAdapterEncodedUTF8(m_interfaceAddress->Description);
    }

    /// @copydoc INetworkInterfaceWrapper::family
    int family() const override
    {
        return m_interfaceFamily;
    }

    /// @copydoc INetworkInterfaceWrapper::address
    std::string address() const override
    {
        std::string retVal;

        if (m_currentUnicastAddress)
        {
            retVal = Utils::IAddressToString(
                this->adapterFamily(),
                (reinterpret_cast<sockaddr_in*>(m_currentUnicastAddress->Address.lpSockaddr))->sin_addr);
        }

        return retVal;
    }

    /// @copydoc INetworkInterfaceWrapper::addressV6
    std::string addressV6() const override
    {
        std::string retVal;

        if (m_currentUnicastAddress)
        {
            if (Utils::isVistaOrLater())
            {
                retVal = Utils::IAddressToString(
                    this->adapterFamily(),
                    (reinterpret_cast<sockaddr_in6*>(m_currentUnicastAddress->Address.lpSockaddr))->sin6_addr);
            }
            else
            {
                // Windows XP
                const auto ipv6Address {reinterpret_cast<sockaddr_in6*>(m_currentUnicastAddress->Address.lpSockaddr)};
                retVal = Utils::getIpV6Address(ipv6Address->sin6_addr.u.Byte);
            }
        }

        return retVal;
    }

    /// @copydoc INetworkInterfaceWrapper::netmask
    std::string netmask() const override
    {
        std::string retVal;

        if (Utils::isVistaOrLater())
        {
            ULONG mask {0};
            static auto pfnGetConvertLengthToIpv4Mask {Utils::getConvertLengthToIpv4MaskFunctionAddress()};

            if (pfnGetConvertLengthToIpv4Mask)
            {
                if (m_currentUnicastAddress &&
                    !pfnGetConvertLengthToIpv4Mask(m_currentUnicastAddress->OnLinkPrefixLength, &mask))
                {
                    retVal = Utils::IAddressToString(this->adapterFamily(), *(reinterpret_cast<in_addr*>(&mask)));
                }
            }
        }
        else
        {
            // Windows XP mechanism
            const auto address {this->address()};
            const auto interfaceAddress {findInterfaceMatch(address)};

            if (interfaceAddress)
            {
                retVal = interfaceAddress->IpMask.String;
            }
        }

        return retVal;
    }

    /// @copydoc INetworkInterfaceWrapper::netmaskV6
    std::string netmaskV6() const override
    {
        std::string retVal;

        if (m_currentUnicastAddress && Utils::isVistaOrLater())
        {
            // Get ipv6Netmask based on current OnLinkPrefixLength value
            retVal = Utils::ipv6Netmask(m_currentUnicastAddress->OnLinkPrefixLength);
        }

        // Windows XP netmask IPv6 is not supported
        return retVal;
    }

    /// @copydoc INetworkInterfaceWrapper::broadcast
    void broadcast(nlohmann::json& network) const override
    {
        network["broadcast"] = UNKNOWN_VALUE;
        const auto address {this->address()};
        const auto netmask {this->netmask()};

        if (address.size() && netmask.size())
        {
            const auto broadcast {Utils::broadcastAddress(address, netmask)};
            if (!broadcast.empty())
            {
                network["broadcast"] = broadcast;
            }
        }
    }

    /// @copydoc INetworkInterfaceWrapper::broadcastV6
    void broadcastV6(nlohmann::json& network) const override
    {
        network["broadcast"] = UNKNOWN_VALUE;
    }

    /// @copydoc INetworkInterfaceWrapper::gateway
    void gateway(nlohmann::json& network) const override
    {
        std::string retVal;
        constexpr auto GATEWAY_SEPARATOR {","};

        if (Utils::isVistaOrLater())
        {
            auto gatewayAddress {m_interfaceAddress->FirstGatewayAddress};

            while (gatewayAddress)
            {
                const auto gatewayFamily {gatewayAddress->Address.lpSockaddr->sa_family};
                const auto sockAddress {gatewayAddress->Address.lpSockaddr};

                if (AF_INET == gatewayFamily)
                {
                    retVal +=
                        Utils::IAddressToString(gatewayFamily, (reinterpret_cast<sockaddr_in*>(sockAddress))->sin_addr);
                    retVal += GATEWAY_SEPARATOR;
                }
                else if (AF_INET6 == gatewayFamily)
                {
                    retVal += Utils::IAddressToString(gatewayFamily,
                                                      (reinterpret_cast<sockaddr_in6*>(sockAddress))->sin6_addr);
                    retVal += GATEWAY_SEPARATOR;
                }

                gatewayAddress = gatewayAddress->Next;
            }
        }
        else
        {
            // Under Windows XP, the only way to retrieve IPv4 gateway addresses is through GetAdaptersInfo()
            PIP_ADDR_STRING currentGWAddress {nullptr};
            PIP_ADAPTER_INFO currentAdapterInfo {m_adapterInfo};
            bool foundMatch {false};

            while (currentAdapterInfo && !foundMatch)
            {
                if (!(MIB_IF_TYPE_LOOPBACK == currentAdapterInfo->Type))
                {
                    if (currentAdapterInfo->Index == m_interfaceAddress->IfIndex)
                    {
                        // Found an interface match.
                        currentGWAddress = &(currentAdapterInfo->GatewayList);

                        while (currentGWAddress)
                        {
                            retVal += currentGWAddress->IpAddress.String;
                            retVal += GATEWAY_SEPARATOR;
                            currentGWAddress = currentGWAddress->Next;
                        }

                        foundMatch = true;
                    }
                }

                currentAdapterInfo = currentAdapterInfo->Next;
            }
        }

        if (retVal.empty())
        {
            network["gateway"] = UNKNOWN_VALUE;
        }
        else
        {
            // Remove last GATEWAY_SEPARATOR (,)
            retVal = retVal.substr(0, retVal.size() - 1);
            network["gateway"] = retVal;
        }
    }

    /// @copydoc INetworkInterfaceWrapper::metrics
    void metrics(nlohmann::json& network) const override
    {
        network["metric"] = UNKNOWN_VALUE;

        if (Utils::isVistaOrLater())
        {
            network["metric"] = std::to_string(m_interfaceAddress->Ipv4Metric);
        }
    }

    /// @copydoc INetworkInterfaceWrapper::metricsV6
    void metricsV6(nlohmann::json& network) const override
    {
        network["metric"] = UNKNOWN_VALUE;

        if (Utils::isVistaOrLater())
        {
            network["metric"] = std::to_string(m_interfaceAddress->Ipv6Metric);
        }
    }

    /// @copydoc INetworkInterfaceWrapper::dhcp
    void dhcp(nlohmann::json& network) const override
    {
        network["dhcp"] = UNKNOWN_VALUE;
        const auto family {this->adapterFamily()};

        if (AF_INET == family)
        {
            const bool ipv4DHCPEnabled {(m_interfaceAddress->Flags & IP_ADAPTER_DHCP_ENABLED) &&
                                        (m_interfaceAddress->Flags & IP_ADAPTER_IPV4_ENABLED)};
            network["dhcp"] = ipv4DHCPEnabled ? "enabled" : "disabled";
        }
        else if (AF_INET6 == family)
        {
            const bool ipv6DHCPEnabled {(m_interfaceAddress->Flags & IP_ADAPTER_DHCP_ENABLED) &&
                                        (m_interfaceAddress->Flags & IP_ADAPTER_IPV6_ENABLED)};
            network["dhcp"] = ipv6DHCPEnabled ? "enabled" : "disabled";
        }
    }

    /// @copydoc INetworkInterfaceWrapper::mtu
    void mtu(nlohmann::json& network) const override
    {
        network["mtu"] = m_interfaceAddress->Mtu;
    }

    /// @copydoc INetworkInterfaceWrapper::stats
    LinkStats stats() const override
    {
        return Utils::isVistaOrLater() ? statsVistaOrLater() : statsXP();
    }

    /// @copydoc INetworkInterfaceWrapper::type
    void type(nlohmann::json& network) const override
    {
        network["type"] = EMPTY_VALUE;
        const auto interfaceType {NETWORK_INTERFACE_TYPES.find(m_interfaceAddress->IfType)};

        if (NETWORK_INTERFACE_TYPES.end() != interfaceType)
        {
            network["type"] = interfaceType->second;
        }
    }

    /// @copydoc INetworkInterfaceWrapper::state
    void state(nlohmann::json& network) const override
    {
        network["state"] = UNKNOWN_VALUE;
        const auto opStatus {NETWORK_OPERATIONAL_STATUS.find(m_interfaceAddress->OperStatus)};

        if (NETWORK_OPERATIONAL_STATUS.end() != opStatus)
        {
            network["state"] = opStatus->second;
        }
    }

    /// @copydoc INetworkInterfaceWrapper::MAC
    void MAC(nlohmann::json& network) const override
    {
        network["mac"] = UNKNOWN_VALUE;
        constexpr auto MAC_ADDRESS_LENGTH {6};

        if (MAC_ADDRESS_LENGTH == m_interfaceAddress->PhysicalAddressLength)
        {
            std::stringstream ss;

            for (unsigned int idx = 0; idx < MAC_ADDRESS_LENGTH; ++idx)
            {
                ss << std::hex << std::setfill('0') << std::setw(2);
                ss << static_cast<int>(static_cast<uint8_t>(m_interfaceAddress->PhysicalAddress[idx]));

                if (MAC_ADDRESS_LENGTH - 1 != idx)
                {
                    ss << ":";
                }
            }

            network["mac"] = ss.str();
        }
    }

private:
    /// @brief Returns UTF-8 encoded adapter name
    /// @param name Adapter name
    /// @return UTF-8 encoded adapter name
    std::string getAdapterEncodedUTF8(const std::wstring& name) const
    {
        return Utils::getAdapterNameStr(name);
    }

    /// @brief Returns interface family
    /// @return Interface family
    int adapterFamily() const
    {
        return m_currentUnicastAddress ? m_currentUnicastAddress->Address.lpSockaddr->sa_family : AF_UNSPEC;
    }

    /// @brief Find interface match
    /// @param address IPv4 address
    /// @return Interface address
    PIP_ADDR_STRING findInterfaceMatch(const std::string& address) const
    {
        PIP_ADDR_STRING currentInterfaceAddr {nullptr};
        PIP_ADAPTER_INFO currentAdapterInfo {m_adapterInfo};
        bool foundMatch {false};

        while (currentAdapterInfo && !foundMatch)
        {
            if (!(MIB_IF_TYPE_LOOPBACK == currentAdapterInfo->Type))
            {
                if (currentAdapterInfo->Index == m_interfaceAddress->IfIndex)
                {
                    // Found an interface match. Now we need an IPv4 match.
                    currentInterfaceAddr = &(currentAdapterInfo->IpAddressList);

                    while (currentInterfaceAddr)
                    {
                        if (strncmp(address.c_str(), currentInterfaceAddr->IpAddress.String, address.length()) == 0)
                        {
                            // IPv4 match found
                            break;
                        }

                        currentInterfaceAddr = currentInterfaceAddr->Next;
                    }

                    foundMatch = true;
                }
            }

            currentAdapterInfo = currentAdapterInfo->Next;
        }

        return currentInterfaceAddr;
    }

    /// @brief Returns link stats for Vista or later
    /// @return Link stats
    LinkStats statsVistaOrLater() const
    {
        LinkStats retVal {};
        auto ifRow {std::make_unique<MIB_IF_ROW2>()};

        if (!ifRow)
        {
            throw std::system_error {static_cast<int>(GetLastError()),
                                     std::system_category(),
                                     "Unable to allocate memory for MIB_IF_ROW2 struct."};
        }

        ifRow->InterfaceIndex =
            (0 != m_interfaceAddress->IfIndex) ? m_interfaceAddress->IfIndex : m_interfaceAddress->Ipv6IfIndex;

        if (0 != ifRow->InterfaceIndex)
        {
            static auto pfnGetIfEntry2 {Utils::getIfEntry2FunctionAddress()};

            if (pfnGetIfEntry2)
            {
                if (NO_ERROR == pfnGetIfEntry2(ifRow.get()))
                {
                    const auto txPackets {ifRow->OutUcastPkts + ifRow->OutNUcastPkts};
                    const auto rxPackets {ifRow->InUcastPkts + ifRow->InNUcastPkts};
                    retVal.txPackets = static_cast<unsigned int>(txPackets);
                    retVal.rxPackets = static_cast<unsigned int>(rxPackets);
                    retVal.txBytes = ifRow->OutOctets;
                    retVal.rxBytes = ifRow->InOctets;
                    retVal.txErrors = static_cast<unsigned int>(ifRow->OutErrors);
                    retVal.rxErrors = static_cast<unsigned int>(ifRow->InErrors);
                    retVal.txDropped = static_cast<unsigned int>(ifRow->OutDiscards);
                    retVal.rxDropped = static_cast<unsigned int>(ifRow->InDiscards);
                }
            }
        }

        return retVal;
    }

    /// @brief Returns link stats for XP or earlier
    /// @return Link stats
    LinkStats statsXP() const
    {
        LinkStats retVal {};
        auto ifRow {std::make_unique<MIB_IFROW>()};

        if (!ifRow)
        {
            throw std::system_error {static_cast<int>(GetLastError()),
                                     std::system_category(),
                                     "Unable to allocate memory for MIB_IFROW struct."};
        }

        ifRow->dwIndex =
            (0 != m_interfaceAddress->IfIndex) ? m_interfaceAddress->IfIndex : m_interfaceAddress->Ipv6IfIndex;

        if (0 != ifRow->dwIndex)
        {
            if (NO_ERROR == GetIfEntry(ifRow.get()))
            {
                const auto txPackets {ifRow->dwOutUcastPkts + ifRow->dwOutNUcastPkts};
                const auto rxPackets {ifRow->dwInUcastPkts + ifRow->dwInNUcastPkts};
                retVal.txPackets = txPackets;
                retVal.rxPackets = rxPackets;
                retVal.txBytes = ifRow->dwOutOctets;
                retVal.rxBytes = ifRow->dwInOctets;
                retVal.txErrors = ifRow->dwOutErrors;
                retVal.rxErrors = ifRow->dwInErrors;
                retVal.txDropped = ifRow->dwOutDiscards;
                retVal.rxDropped = ifRow->dwInDiscards;
            }
        }

        return retVal;
    }

    Utils::NetworkFamilyTypes m_interfaceFamily;
    PIP_ADAPTER_ADDRESSES m_interfaceAddress;
    PIP_ADAPTER_UNICAST_ADDRESS m_currentUnicastAddress;
    PIP_ADAPTER_INFO m_adapterInfo; // XP needed structure
};
