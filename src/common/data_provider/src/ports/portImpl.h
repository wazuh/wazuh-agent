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

#ifndef _PORT_IMPL_H
#define _PORT_IMPL_H

#include "iportInterface.h"
#include "iportWrapper.h"
#include "sharedDefs.h"

class PortImpl final : public IOSPort
{
    private:
        std::shared_ptr<IPortWrapper> m_spPortRawData;
    public:
        explicit PortImpl(const std::shared_ptr<IPortWrapper>& portRawData)
            : m_spPortRawData(portRawData)
        { }
        // LCOV_EXCL_START
        ~PortImpl() = default;
        // LCOV_EXCL_STOP
        void buildPortData(nlohmann::json& port) override
        {
            m_spPortRawData->protocol(port);
            m_spPortRawData->localIp(port);
            m_spPortRawData->localPort(port);
            m_spPortRawData->remoteIP(port);
            m_spPortRawData->remotePort(port);
            m_spPortRawData->txQueue(port);
            m_spPortRawData->rxQueue(port);
            m_spPortRawData->inode(port);
            m_spPortRawData->state(port);
            m_spPortRawData->pid(port);
            m_spPortRawData->processName(port);
        }
};
#endif // _PORT_IMPL_H
