/*
 * Wazuh SYSINFO
 * Copyright (C) 2015, Wazuh Inc.
 * October 24, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include <ifaddrs.h>
#include "networkInterfaceBSD.h"
#include "networkBSDWrapper.h"

std::shared_ptr<IOSNetwork> FactoryBSDNetwork::create(const std::shared_ptr<INetworkInterfaceWrapper>& interfaceWrapper)
{
    std::shared_ptr<IOSNetwork> ret;

    if (interfaceWrapper)
    {
        const auto family { interfaceWrapper->family() };

        if (AF_INET == family)
        {
            ret = std::make_shared<BSDNetworkImpl<AF_INET>>(interfaceWrapper);
        }
        else if (AF_INET6 == family)
        {
            ret = std::make_shared<BSDNetworkImpl<AF_INET6>>(interfaceWrapper);
        }
        else if (AF_LINK == family)
        {
            ret = std::make_shared<BSDNetworkImpl<AF_LINK>>(interfaceWrapper);
        }

        // else: The current interface family is not supported
    }
    else
    {
        throw std::runtime_error { "Error nullptr interfaceWrapper instance." };
    }

    return ret;
}

template <>
void BSDNetworkImpl<AF_INET>::buildNetworkData(nlohmann::json& network)
{
    // Get IPv4 address
    const auto address { m_interfaceAddress->address() };

    if (!address.empty())
    {
        nlohmann::json ipv4JS {};
        ipv4JS["address"] = address;
        ipv4JS["netmask"] = m_interfaceAddress->netmask();
        m_interfaceAddress->broadcast(ipv4JS);
        m_interfaceAddress->metrics(ipv4JS);
        m_interfaceAddress->dhcp(ipv4JS);

        network["IPv4"].push_back(ipv4JS);
    }
    else
    {
        throw std::runtime_error { "Invalid IpV4 address." };
    }
}
template <>
void BSDNetworkImpl<AF_INET6>::buildNetworkData(nlohmann::json& network)
{
    const auto address { m_interfaceAddress->addressV6() };

    if (!address.empty())
    {
        nlohmann::json ipv6JS {};
        ipv6JS["address"] = address;
        ipv6JS["netmask"] = m_interfaceAddress->netmaskV6();
        m_interfaceAddress->broadcastV6(ipv6JS);
        m_interfaceAddress->metricsV6(ipv6JS);
        m_interfaceAddress->dhcp(ipv6JS);

        network["IPv6"].push_back(ipv6JS);
    }
    else
    {
        throw std::runtime_error { "Invalid IpV4 address." };
    }
}
template <>
void BSDNetworkImpl<AF_LINK>::buildNetworkData(nlohmann::json& network)
{
    /* Get stats of interface */

    network["name"] = m_interfaceAddress->name();
    m_interfaceAddress->adapter(network);
    m_interfaceAddress->state(network);
    m_interfaceAddress->type(network);
    m_interfaceAddress->MAC(network);

    const auto stats { m_interfaceAddress->stats() };

    network["tx_packets"] = stats.txPackets;
    network["rx_packets"] = stats.rxPackets;
    network["tx_bytes"] = stats.txBytes;
    network["rx_bytes"] = stats.rxBytes;
    network["tx_errors"] = stats.txErrors;
    network["rx_errors"] = stats.rxErrors;
    network["tx_dropped"] = stats.txDropped;
    network["rx_dropped"] = stats.rxDropped;

    m_interfaceAddress->mtu(network);
    m_interfaceAddress->gateway(network);
}
