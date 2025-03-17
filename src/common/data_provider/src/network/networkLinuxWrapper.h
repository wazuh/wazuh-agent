#pragma once

#include "inetworkWrapper.h"
#include "networkHelper.hpp"
#include "sharedDefs.h"
#include "stringHelper.hpp"
#include <file_io_utils.hpp>
#include <ifaddrs.h>
#include <net/if_arp.h>
#include <sys/socket.h>

#ifndef ARPHRD_TUNNEL
#define ARPHRD_TUNNEL 768 /* IPIP tunnel.  */
#endif
#ifndef ARPHRD_TUNNEL6
#define ARPHRD_TUNNEL6 769 /* IPIP6 tunnel.  */
#endif
#ifndef ARPHRD_FRAD
#define ARPHRD_FRAD 770 /* Frame Relay Access Device.  */
#endif
#ifndef ARPHRD_SKIP
#define ARPHRD_SKIP 771 /* SKIP vif.  */
#endif
#ifndef ARPHRD_LOOPBACK
#define ARPHRD_LOOPBACK 772 /* Loopback device.  */
#endif
#ifndef ARPHRD_LOCALTLK
#define ARPHRD_LOCALTLK 773 /* Localtalk device.  */
#endif
#ifndef ARPHRD_FDDI
#define ARPHRD_FDDI 774 /* Fiber Distributed Data Interface. */
#endif
#ifndef ARPHRD_BIF
#define ARPHRD_BIF 775 /* AP1000 BIF.  */
#endif
#ifndef ARPHRD_SIT
#define ARPHRD_SIT 776 /* sit0 device - IPv6-in-IPv4.  */
#endif
#ifndef ARPHRD_IPDDP
#define ARPHRD_IPDDP 777 /* IP-in-DDP tunnel.  */
#endif
#ifndef ARPHRD_IPGRE
#define ARPHRD_IPGRE 778 /* GRE over IP.  */
#endif
#ifndef ARPHRD_PIMREG
#define ARPHRD_PIMREG 779 /* PIMSM register interface.  */
#endif
#ifndef ARPHRD_HIPPI
#define ARPHRD_HIPPI 780 /* High Performance Parallel I'face. */
#endif
#ifndef ARPHRD_ASH
#define ARPHRD_ASH 781 /* (Nexus Electronics) Ash.  */
#endif
#ifndef ARPHRD_ECONET
#define ARPHRD_ECONET 782 /* Acorn Econet.  */
#endif
#ifndef ARPHRD_IRDA
#define ARPHRD_IRDA 783 /* Linux-IrDA.  */
#endif
#ifndef ARPHRD_FCPP
#define ARPHRD_FCPP 784 /* Point to point fibrechanel.  */
#endif
#ifndef ARPHRD_FCAL
#define ARPHRD_FCAL 785 /* Fibrechanel arbitrated loop.  */
#endif
#ifndef ARPHRD_FCPL
#define ARPHRD_FCPL 786 /* Fibrechanel public loop.  */
#endif
#ifndef ARPHRD_FCFABRIC
#define ARPHRD_FCFABRIC 787 /* Fibrechanel fabric.  */
#endif
#ifndef ARPHRD_IEEE802_TR
#define ARPHRD_IEEE802_TR 800 /* Magic type ident for TR.  */
#endif
#ifndef ARPHRD_IEEE80211
#define ARPHRD_IEEE80211 801 /* IEEE 802.11.  */
#endif
#ifndef ARPHRD_IEEE80211_PRISM
#define ARPHRD_IEEE80211_PRISM 802 /* IEEE 802.11 + Prism2 header.  */
#endif
#ifndef ARPHRD_IEEE80211_RADIOTAP
#define ARPHRD_IEEE80211_RADIOTAP 803 /* IEEE 802.11 + radiotap header.  */
#endif
#ifndef ARPHRD_IEEE802154
#define ARPHRD_IEEE802154 804 /* IEEE 802.15.4 header.  */
#endif
#ifndef ARPHRD_IEEE802154_PHY
#define ARPHRD_IEEE802154_PHY 805 /* IEEE 802.15.4 PHY header.  */
#endif

static const std::map<std::pair<int, int>, std::string> NETWORK_INTERFACE_TYPE = {
    {std::make_pair(ARPHRD_ETHER, ARPHRD_ETHER), "ethernet"},
    {std::make_pair(ARPHRD_PRONET, ARPHRD_PRONET), "token ring"},
    {std::make_pair(ARPHRD_PPP, ARPHRD_PPP), "point-to-point"},
    {std::make_pair(ARPHRD_ATM, ARPHRD_ATM), "ATM"},
    {std::make_pair(ARPHRD_IEEE1394, ARPHRD_IEEE1394), "firewire"},
    {std::make_pair(ARPHRD_TUNNEL, ARPHRD_IRDA), "tunnel"},
    {std::make_pair(ARPHRD_FCPP, ARPHRD_FCFABRIC), "fibrechannel"},
    {std::make_pair(ARPHRD_IEEE802_TR, ARPHRD_IEEE802154_PHY), "wireless"},
};

static const std::map<std::string, std::string> DHCP_STATUS = {
    {"dhcp", "enabled"},
    {"yes", "enabled"},
    {"static", "disabled"},
    {"none", "disabled"},
    {"no", "disabled"},
    {"manual", "disabled"},
    {"bootp", "BOOTP"},
};

namespace GatewayFileFields
{
    enum
    {
        Iface,
        Destination,
        Gateway,
        Flags,
        RefCnt,
        Use,
        Metric,
        Mask,
        MTU,
        Window,
        IRTT,
        Size
    };
}

namespace DebianInterfaceConfig
{
    enum Config
    {
        Type,
        Name,
        Family,
        Method,
        Size
    };
}

namespace RHInterfaceConfig
{
    enum Config
    {
        Key,
        Value,
        Size
    };
}

namespace NetDevFileFields
{
    enum
    {
        Iface,
        RxBytes,
        RxPackets,
        RxErrors,
        RxDropped,
        RxFifo,
        RxFrame,
        RxCompressed,
        RxMulticast,
        TxBytes,
        TxPackets,
        TxErrors,
        TxDropped,
        TxFifo,
        TxColls,
        TxCarrier,
        TxCompressed,
        FieldsQuantity
    };
}

/// @brief Class to work with network interface
class NetworkLinuxInterface final : public INetworkInterfaceWrapper
{
    ifaddrs* m_interfaceAddress;
    std::string m_gateway;
    std::string m_metrics;

    /// @brief Get network interface name
    /// @param inputData interface address
    /// @param socketLen socket length
    /// @return the name
    static std::string getNameInfo(const sockaddr* inputData, const socklen_t socketLen)
    {
        auto retVal {std::make_unique<char[]>(NI_MAXHOST)};

        if (inputData)
        {
            const auto result {getnameinfo(inputData, socketLen, retVal.get(), NI_MAXHOST, NULL, 0, NI_NUMERICHOST)};

            if (result != 0)
            {
                throw std::runtime_error {"Cannot get socket address information, Code: " + std::to_string(result)};
            }
        }

        return retVal.get();
    }

    /// @brief Get DHCP status from redhat
    /// @param fields interface configuration
    /// @return the status
    static std::string getRedHatDHCPStatus(const std::vector<std::string>& fields)
    {
        std::string retVal {"enabled"};
        const auto value {fields.at(RHInterfaceConfig::Value)};

        const auto it {DHCP_STATUS.find(value)};

        if (DHCP_STATUS.end() != it)
        {
            retVal = it->second;
        }

        return retVal;
    }

    /// @brief Get DHCP status from debian
    /// @param family interface family
    /// @param fields interface configuration
    /// @return the status
    static std::string getDebianDHCPStatus(const std::string& family, const std::vector<std::string>& fields)
    {
        std::string retVal {"enabled"};

        if (fields.at(DebianInterfaceConfig::Family).compare(family) == 0)
        {
            const auto method {fields.at(DebianInterfaceConfig::Method)};

            const auto it {DHCP_STATUS.find(method)};

            if (DHCP_STATUS.end() != it)
            {
                retVal = it->second;
            }
        }

        return retVal;
    }

public:
    /// @brief Constructor
    /// @param addrs interface address
    explicit NetworkLinuxInterface(ifaddrs* addrs)
        : m_interfaceAddress {addrs}
        , m_gateway {}
    {
        if (!addrs)
        {
            throw std::runtime_error {"Nullptr instances of network interface"};
        }
        else
        {
            const auto fileIoWrapper = std::make_unique<file_io::FileIOUtils>();
            auto fileData {fileIoWrapper->getFileContent(std::string(WM_SYS_NET_DIR) + "route")};
            const auto ifName {this->name()};

            if (!fileData.empty())
            {
                auto lines {Utils::split(fileData, '\n')};

                for (auto& line : lines)
                {
                    line = Utils::RightTrim(line);
                    Utils::replaceAll(line, "\t", " ");
                    Utils::replaceAll(line, "  ", " ");
                    const auto fields {Utils::split(line, ' ')};

                    if (GatewayFileFields::Size == fields.size() &&
                        fields.at(GatewayFileFields::Iface).compare(ifName) == 0)
                    {
                        auto address {static_cast<uint32_t>(std::stol(fields.at(GatewayFileFields::Gateway), 0, 16))};
                        m_metrics = fields.at(GatewayFileFields::Metric);

                        if (address)
                        {
                            m_gateway = Utils::IAddressToBinary(AF_INET, reinterpret_cast<in_addr*>(&address));
                            break;
                        }
                    }
                }
            }
        }
    }

    /// @copydoc INetworkInterfaceWrapper::name
    std::string name() const override
    {
        return m_interfaceAddress->ifa_name ? Utils::substrOnFirstOccurrence(m_interfaceAddress->ifa_name, ":")
                                            : EMPTY_VALUE;
    }

    /// @copydoc INetworkInterfaceWrapper::adapter
    void adapter(nlohmann::json& network) const override
    {
        network["adapter"] = EMPTY_VALUE;
    }

    /// @copydoc INetworkInterfaceWrapper::family
    int family() const override
    {
        return m_interfaceAddress->ifa_addr ? m_interfaceAddress->ifa_addr->sa_family : AF_PACKET;
    }

    /// @copydoc INetworkInterfaceWrapper::address
    std::string address() const override
    {
        return m_interfaceAddress->ifa_addr ? getNameInfo(m_interfaceAddress->ifa_addr, sizeof(struct sockaddr_in))
                                            : EMPTY_VALUE;
    }

    /// @copydoc INetworkInterfaceWrapper::netmask
    std::string netmask() const override
    {
        return m_interfaceAddress->ifa_netmask
                   ? getNameInfo(m_interfaceAddress->ifa_netmask, sizeof(struct sockaddr_in))
                   : EMPTY_VALUE;
    }

    /// @copydoc INetworkInterfaceWrapper::broadcast
    void broadcast(nlohmann::json& network) const override
    {
        network["broadcast"] = UNKNOWN_VALUE;

        if (m_interfaceAddress->ifa_ifu.ifu_broadaddr)
        {
            network["broadcast"] = getNameInfo(m_interfaceAddress->ifa_ifu.ifu_broadaddr, sizeof(struct sockaddr_in));
        }
        else
        {
            const auto netmask {this->netmask()};
            const auto address {this->address()};

            if (address.size() && netmask.size())
            {
                const auto broadcast {Utils::getBroadcast(address, netmask)};
                if (!broadcast.empty())
                {
                    network["broadcast"] = broadcast;
                }
            }
        }
    }

    /// @copydoc INetworkInterfaceWrapper::addressV6
    std::string addressV6() const override
    {
        return m_interfaceAddress->ifa_addr
                   ? Utils::splitIndex(getNameInfo(m_interfaceAddress->ifa_addr, sizeof(struct sockaddr_in6)), '%', 0)
                   : EMPTY_VALUE;
    }

    /// @copydoc INetworkInterfaceWrapper::netmaskV6
    std::string netmaskV6() const override
    {
        return m_interfaceAddress->ifa_netmask
                   ? getNameInfo(m_interfaceAddress->ifa_netmask, sizeof(struct sockaddr_in6))
                   : EMPTY_VALUE;
    }

    /// @copydoc INetworkInterfaceWrapper::broadcastV6
    void broadcastV6(nlohmann::json& network) const override
    {
        m_interfaceAddress->ifa_ifu.ifu_broadaddr
            ? network["broadcast"] = getNameInfo(m_interfaceAddress->ifa_ifu.ifu_broadaddr, sizeof(struct sockaddr_in6))
            : network["broadcast"] = UNKNOWN_VALUE;
    }

    /// @copydoc INetworkInterfaceWrapper::gateway
    void gateway(nlohmann::json& network) const override
    {
        network["gateway"] = m_gateway;
    }

    /// @copydoc INetworkInterfaceWrapper::metrics
    void metrics(nlohmann::json& network) const override
    {
        network["metric"] = m_metrics;
    }

    /// @copydoc INetworkInterfaceWrapper::metricsV6
    void metricsV6(nlohmann::json& network) const override
    {
        network["metric"] = UNKNOWN_VALUE;
    }

    /// @copydoc INetworkInterfaceWrapper::dhcp
    void dhcp(nlohmann::json& network) const override
    {
        const auto fileIoWrapper = std::make_unique<file_io::FileIOUtils>();
        auto fileData {fileIoWrapper->getFileContent(WM_SYS_IF_FILE)};
        network["dhcp"] = UNKNOWN_VALUE;
        const auto family {this->family()};
        const auto ifName {this->name()};

        if (!fileData.empty())
        {
            const auto lines {Utils::split(fileData, '\n')};

            for (const auto& line : lines)
            {
                const auto fields {Utils::split(line, ' ')};

                if (DebianInterfaceConfig::Size == fields.size())
                {
                    if (fields.at(DebianInterfaceConfig::Type).compare("iface") == 0 &&
                        fields.at(DebianInterfaceConfig::Name).compare(ifName) == 0)
                    {
                        if (AF_INET == family)
                        {
                            network["dhcp"] = getDebianDHCPStatus("inet", fields);
                            break;
                        }
                        else if (AF_INET6 == family)
                        {
                            network["dhcp"] = getDebianDHCPStatus("inet6", fields);
                            break;
                        }
                    }
                }
            }
        }
        else
        {
            const auto fileName {"ifcfg-" + ifName};
            fileData = fileIoWrapper->getFileContent(WM_SYS_IF_DIR_RH + fileName);
            fileData = fileData.empty() ? fileIoWrapper->getFileContent(WM_SYS_IF_DIR_SUSE + fileName) : fileData;

            if (!fileData.empty())
            {
                const auto lines {Utils::split(fileData, '\n')};

                for (const auto& line : lines)
                {
                    const auto fields {Utils::split(line, '=')};

                    if (fields.size() == RHInterfaceConfig::Size)
                    {
                        if (AF_INET == family)
                        {
                            if (fields.at(RHInterfaceConfig::Key).compare("BOOTPROTO") == 0)
                            {
                                network["dhcp"] = getRedHatDHCPStatus(fields);
                                break;
                            }
                        }
                        else if (AF_INET6 == family)
                        {
                            if (fields.at(RHInterfaceConfig::Key).compare("DHCPV6C") == 0)
                            {
                                network["dhcp"] = getRedHatDHCPStatus(fields);
                                break;
                            }
                        }
                    }
                }
            }
        }
    }

    /// @copydoc INetworkInterfaceWrapper::mtu
    void mtu(nlohmann::json& network) const override
    {
        network["mtu"] = UNKNOWN_VALUE;
        const auto fileIoWrapper = std::make_unique<file_io::FileIOUtils>();
        const auto mtuFileContent {
            fileIoWrapper->getFileContent(std::string(WM_SYS_IFDATA_DIR) + this->name() + "/mtu")};

        if (!mtuFileContent.empty())
        {
            network["mtu"] = std::stol(Utils::splitIndex(mtuFileContent, '\n', 0));
        }
    }

    /// @copydoc INetworkInterfaceWrapper::stats
    LinkStats stats() const override
    {
        LinkStats retVal {};

        try
        {
            const auto fileIoWrapper = std::make_unique<file_io::FileIOUtils>();
            const auto devData {fileIoWrapper->getFileContent(std::string(WM_SYS_NET_DIR) + "dev")};

            if (!devData.empty())
            {
                auto lines {Utils::split(devData, '\n')};
                lines.erase(lines.begin());
                lines.erase(lines.begin());

                for (auto& line : lines)
                {
                    line = Utils::Trim(line);
                    Utils::replaceAll(line, "\t", " ");
                    Utils::replaceAll(line, "  ", " ");
                    Utils::replaceAll(line, ": ", " ");
                    const auto fields {Utils::split(line, ' ')};

                    if (NetDevFileFields::FieldsQuantity == fields.size())
                    {
                        if (fields.at(NetDevFileFields::Iface).compare(this->name()) == 0)
                        {
                            retVal.rxBytes = std::stoul(fields.at(NetDevFileFields::RxBytes));
                            retVal.txBytes = std::stoul(fields.at(NetDevFileFields::TxBytes));
                            retVal.rxPackets = std::stoul(fields.at(NetDevFileFields::RxPackets));
                            retVal.txPackets = std::stoul(fields.at(NetDevFileFields::TxPackets));
                            retVal.rxErrors = std::stoul(fields.at(NetDevFileFields::RxErrors));
                            retVal.txErrors = std::stoul(fields.at(NetDevFileFields::TxErrors));
                            retVal.rxDropped = std::stoul(fields.at(NetDevFileFields::RxDropped));
                            retVal.txDropped = std::stoul(fields.at(NetDevFileFields::TxDropped));
                            break;
                        }
                    }
                }
            }
        }
        catch (...)
        {
        }

        return retVal;
    }

    /// @copydoc INetworkInterfaceWrapper::type
    void type(nlohmann::json& network) const override
    {
        network["type"] = EMPTY_VALUE;
        const auto fileIoWrapper = std::make_unique<file_io::FileIOUtils>();
        const auto networkTypeCode {
            fileIoWrapper->getFileContent(std::string(WM_SYS_IFDATA_DIR) + this->name() + "/type")};

        if (!networkTypeCode.empty())
        {
            network["type"] = Utils::getNetworkTypeStringCode(std::stoi(networkTypeCode), NETWORK_INTERFACE_TYPE);
        }
    }

    /// @copydoc INetworkInterfaceWrapper::state
    void state(nlohmann::json& network) const override
    {
        network["state"] = UNKNOWN_VALUE;
        const auto fileIoWrapper = std::make_unique<file_io::FileIOUtils>();
        const std::string operationalState {
            fileIoWrapper->getFileContent(std::string(WM_SYS_IFDATA_DIR) + this->name() + "/operstate")};

        if (!operationalState.empty())
        {
            network["state"] = Utils::splitIndex(operationalState, '\n', 0);
        }
    }

    /// @copydoc INetworkInterfaceWrapper::MAC
    void MAC(nlohmann::json& network) const override
    {
        network["mac"] = UNKNOWN_VALUE;
        const auto fileIoWrapper = std::make_unique<file_io::FileIOUtils>();
        const std::string macContent {
            fileIoWrapper->getFileContent(std::string(WM_SYS_IFDATA_DIR) + this->name() + "/address")};

        if (!macContent.empty())
        {
            network["mac"] = Utils::splitIndex(macContent, '\n', 0);
        }
    }
};
