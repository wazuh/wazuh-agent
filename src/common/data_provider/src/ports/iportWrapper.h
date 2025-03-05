/*
 * Wazuh SYSINFO
 * Copyright (C) 2015, Wazuh Inc.
 * November 3, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _PORT_WRAPPER_H
#define _PORT_WRAPPER_H

#include "iportInterface.h"

enum LinuxPortsFieldsData
{
    ENTRY,
    LOCAL_ADDRESS,
    REMOTE_ADDRESS,
    STATE,
    QUEUE,
    TIMER_ACTIVE,
    RETRANSMITION,
    UID,
    TIMEOUT,
    INODE,
    SIZE_LINUX_PORT_FIELDS
};

class IPortWrapper
{
public:
    // LCOV_EXCL_START
    virtual ~IPortWrapper() = default;
    // LCOV_EXCL_STOP
    virtual void protocol(nlohmann::json&) const = 0;
    virtual void localIp(nlohmann::json&) const = 0;
    virtual void localPort(nlohmann::json&) const = 0;
    virtual void remoteIP(nlohmann::json&) const = 0;
    virtual void remotePort(nlohmann::json&) const = 0;
    virtual void txQueue(nlohmann::json&) const = 0;
    virtual void rxQueue(nlohmann::json&) const = 0;
    virtual void inode(nlohmann::json&) const = 0;
    virtual void state(nlohmann::json&) const = 0;
    virtual void pid(nlohmann::json&) const = 0;
    virtual void processName(nlohmann::json&) const = 0;
};
#endif // _PORT_WRAPPER_H
