/*
 * Wazuh SYSINFO
 * Copyright (C) 2015, Wazuh Inc.
 * October 26, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _NETWORK_INTERFACE_WRAPPER_H
#define _NETWORK_INTERFACE_WRAPPER_H

#include "inetworkInterface.h"
#include <optional>

class INetworkInterfaceWrapper
{
public:
    // LCOV_EXCL_START
    virtual ~INetworkInterfaceWrapper() = default;
    // LCOV_EXCL_STOP
    virtual int family() const = 0;
    virtual std::string name() const = 0;
    virtual void adapter(nlohmann::json&) const = 0;
    virtual std::string address() const = 0;
    virtual std::string netmask() const = 0;
    virtual void broadcast(nlohmann::json&) const = 0;
    virtual std::string addressV6() const = 0;
    virtual std::string netmaskV6() const = 0;
    virtual void broadcastV6(nlohmann::json&) const = 0;
    virtual void gateway(nlohmann::json&) const = 0;
    virtual void metrics(nlohmann::json&) const = 0;
    virtual void metricsV6(nlohmann::json&) const = 0;
    virtual void dhcp(nlohmann::json&) const = 0;
    virtual void mtu(nlohmann::json&) const = 0;
    virtual LinkStats stats() const = 0;
    virtual void type(nlohmann::json&) const = 0;
    virtual void state(nlohmann::json&) const = 0;
    virtual void MAC(nlohmann::json&) const = 0;
};
#endif // _NETWORK_INTERFACE_WRAPPER_H
