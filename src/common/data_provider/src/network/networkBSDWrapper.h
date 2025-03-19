#pragma once

#include "inetworkWrapper.h"
#include "networkHelper.hpp"
#include "sharedDefs.h"
#include "stringHelper.hpp"
#include <ifaddrs.h>
#include <iomanip>
#include <net/if.h>
#include <net/if_dl.h>
#include <net/if_types.h>
#include <net/route.h>
#include <netinet/in.h>
#include <sstream>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/sysctl.h>

static const std::map<std::pair<int, int>, std::string> NETWORK_INTERFACE_TYPE = {
    {std::make_pair(IFT_ETHER, IFT_ETHER), "ethernet"},
    {std::make_pair(IFT_ISO88023, IFT_ISO88023), "CSMA/CD"},
    {std::make_pair(IFT_ISO88024, IFT_ISO88025), "token ring"},
    {std::make_pair(IFT_FDDI, IFT_FDDI), "FDDI"},
    {std::make_pair(IFT_PPP, IFT_PPP), "point-to-point"},
    {std::make_pair(IFT_ATM, IFT_ATM), "ATM"},
};

/// @brief NetworkBSD interface
class NetworkBSDInterface final : public INetworkInterfaceWrapper
{
    ifaddrs* m_interfaceAddress;
    const std::string m_scanTime;

public:
    /// @brief NetworkBSD interface
    /// @param addrs interface address
    explicit NetworkBSDInterface(ifaddrs* addrs)
        : m_interfaceAddress {addrs}
    {
        if (!addrs)
        {
            throw std::runtime_error {"Nullptr instances of network interface"};
        }
    }

    /// @copydoc INetworkInterfaceWrapper::name
    std::string name() const override
    {
        return m_interfaceAddress->ifa_name ? Utils::substrOnFirstOccurrence(m_interfaceAddress->ifa_name, ":") : "";
    }

    /// @copydoc INetworkInterfaceWrapper::adapter
    void adapter(nlohmann::json& network) const override
    {
        network["adapter"] = EMPTY_VALUE;
    }

    /// @copydoc INetworkInterfaceWrapper::family
    int family() const override
    {
        return m_interfaceAddress->ifa_addr ? m_interfaceAddress->ifa_addr->sa_family : AF_UNSPEC;
    }

    /// @copydoc INetworkInterfaceWrapper::address
    std::string address() const override
    {
        return m_interfaceAddress->ifa_addr
                   ? Utils::IAddressToBinary(this->family(),
                                             &(reinterpret_cast<sockaddr_in*>(m_interfaceAddress->ifa_addr))->sin_addr)
                   : "";
    }

    /// @copydoc INetworkInterfaceWrapper::netmask
    std::string netmask() const override
    {
        return m_interfaceAddress->ifa_netmask
                   ? Utils::IAddressToBinary(
                         m_interfaceAddress->ifa_netmask->sa_family,
                         &(reinterpret_cast<sockaddr_in*>(m_interfaceAddress->ifa_netmask))->sin_addr)
                   : "";
    }

    /// @copydoc INetworkInterfaceWrapper::broadcast
    void broadcast(nlohmann::json& network) const override
    {
        network["broadcast"] =
            m_interfaceAddress->ifa_dstaddr
                ? Utils::IAddressToBinary(m_interfaceAddress->ifa_dstaddr->sa_family,
                                          &(reinterpret_cast<sockaddr_in*>(m_interfaceAddress->ifa_dstaddr))->sin_addr)
                : EMPTY_VALUE;
    }

    /// @copydoc INetworkInterfaceWrapper::addressV6
    std::string addressV6() const override
    {
        return m_interfaceAddress->ifa_addr
                   ? Utils::IAddressToBinary(
                         m_interfaceAddress->ifa_addr->sa_family,
                         &(reinterpret_cast<sockaddr_in6*>(m_interfaceAddress->ifa_addr))->sin6_addr)
                   : "";
    }

    /// @copydoc INetworkInterfaceWrapper::netmaskV6
    std::string netmaskV6() const override
    {
        return m_interfaceAddress->ifa_netmask
                   ? Utils::IAddressToBinary(
                         m_interfaceAddress->ifa_netmask->sa_family,
                         &(reinterpret_cast<sockaddr_in6*>(m_interfaceAddress->ifa_netmask))->sin6_addr)
                   : "";
    }

    /// @copydoc INetworkInterfaceWrapper::broadcastV6
    void broadcastV6(nlohmann::json& network) const override
    {
        network["broadcast"] = m_interfaceAddress->ifa_dstaddr
                                   ? Utils::IAddressToBinary(
                                         m_interfaceAddress->ifa_dstaddr->sa_family,
                                         &(reinterpret_cast<sockaddr_in6*>(m_interfaceAddress->ifa_dstaddr))->sin6_addr)
                                   : EMPTY_VALUE;
    }

    /// @copydoc INetworkInterfaceWrapper::gateway
    void gateway(nlohmann::json& network) const override
    {
        network["gateway"] = UNKNOWN_VALUE;
        size_t tableSize {0};
        int mib[] = {CTL_NET, AF_ROUTE, 0, AF_UNSPEC, NET_RT_FLAGS, RTF_UP | RTF_GATEWAY};

        if (sysctl(mib, sizeof(mib) / sizeof(int), nullptr, &tableSize, nullptr, 0) == 0)
        {
            std::unique_ptr<char[]> table {std::make_unique<char[]>(tableSize)};

            if (sysctl(mib, sizeof(mib) / sizeof(int), table.get(), &tableSize, nullptr, 0) == 0)
            {
                size_t messageLength {0};

                for (char* p = table.get(); p < table.get() + tableSize; p += messageLength)
                {
                    auto msg {reinterpret_cast<rt_msghdr*>(p)};
                    auto sa {reinterpret_cast<sockaddr*>(msg + 1)};
                    auto sdl {reinterpret_cast<sockaddr_dl*>(m_interfaceAddress->ifa_addr)};

                    if (sdl && (msg->rtm_addrs & RTA_GATEWAY) == RTA_GATEWAY && msg->rtm_index == sdl->sdl_index)
                    {
                        auto sock {reinterpret_cast<sockaddr*>(reinterpret_cast<char*>(sa) + ROUNDUP(sa->sa_len))};

                        if (sock && AF_INET == sock->sa_family)
                        {
                            network["gateway"] =
                                Utils::IAddressToBinary(AF_INET, &reinterpret_cast<sockaddr_in*>(sock)->sin_addr);
                        }

                        break;
                    }

                    messageLength = msg->rtm_msglen;
                }
            }
        }
    }

    /// @copydoc INetworkInterfaceWrapper::metrics
    void metrics(nlohmann::json& network) const override
    {
        network["metric"] = UNKNOWN_VALUE;
    }

    /// @copydoc INetworkInterfaceWrapper::metricsV6
    void metricsV6(nlohmann::json& network) const override
    {
        network["metric"] = UNKNOWN_VALUE;
    }

    /// @copydoc INetworkInterfaceWrapper::dhcp
    void dhcp(nlohmann::json& network) const override
    {
        network["dhcp"] = UNKNOWN_VALUE;
    }

    /// @copydoc INetworkInterfaceWrapper::mtu
    void mtu(nlohmann::json& network) const override
    {
        network["mtu"] = UNKNOWN_VALUE;
        if (m_interfaceAddress->ifa_data)
        {
            network["mtu"] = reinterpret_cast<if_data*>(m_interfaceAddress->ifa_data)->ifi_mtu;
        }
    }

    /// @copydoc INetworkInterfaceWrapper::stats
    LinkStats stats() const override
    {
        const auto stats {reinterpret_cast<if_data*>(m_interfaceAddress->ifa_data)};
        LinkStats retVal {};

        if (stats)
        {
            retVal.txPackets = stats->ifi_opackets;
            retVal.rxPackets = stats->ifi_ipackets;
            retVal.txBytes = stats->ifi_obytes;
            retVal.rxBytes = stats->ifi_ibytes;
            retVal.txErrors = stats->ifi_oerrors;
            retVal.rxErrors = stats->ifi_ierrors;
            retVal.rxDropped = stats->ifi_iqdrops;
        }

        return retVal;
    }

    /// @copydoc INetworkInterfaceWrapper::type
    void type(nlohmann::json& network) const override
    {
        network["type"] = EMPTY_VALUE;

        if (m_interfaceAddress->ifa_addr)
        {
            auto sdl {reinterpret_cast<struct sockaddr_dl*>(m_interfaceAddress->ifa_addr)};
            const auto type {Utils::getNetworkTypeStringCode(sdl->sdl_type, NETWORK_INTERFACE_TYPE)};
            if (!type.empty())
            {
                network["type"] = type;
            }
        }
    }

    /// @copydoc INetworkInterfaceWrapper::state
    void state(nlohmann::json& network) const override
    {
        network["state"] = m_interfaceAddress->ifa_flags & IFF_UP ? "up" : "down";
    }

    /// @copydoc INetworkInterfaceWrapper::MAC
    void MAC(nlohmann::json& network) const override
    {
        network["mac"] = UNKNOWN_VALUE;
        auto sdl {reinterpret_cast<struct sockaddr_dl*>(m_interfaceAddress->ifa_addr)};
        std::stringstream ss;

        if (sdl && MAC_ADDRESS_COUNT_SEGMENTS == sdl->sdl_alen)
        {
            auto macAddress {&sdl->sdl_data[sdl->sdl_nlen]};

            if (macAddress)
            {
                for (auto i = 0ull; i < MAC_ADDRESS_COUNT_SEGMENTS; ++i)
                {
                    ss << std::hex << std::setfill('0') << std::setw(2);
                    ss << static_cast<int>(static_cast<uint8_t>(macAddress[i]));

                    if (i != MAC_ADDRESS_COUNT_SEGMENTS - 1)
                    {
                        ss << ":";
                    }
                }

                network["mac"] = ss.str();
            }
        }
    }
};
