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

#ifndef _PORT_WINDOWS_WRAPPER_H
#define _PORT_WINDOWS_WRAPPER_H

#include "iportWrapper.h"
#include "sharedDefs.h"
#include "stringHelper.hpp"
#include "windowsHelper.hpp"
#include <pal.h>
#include <ws2ipdef.h>

static const std::map<int32_t, std::string> STATE_TYPE = {{MIB_TCP_STATE_ESTAB, "established"},
                                                          {MIB_TCP_STATE_SYN_SENT, "syn_sent"},
                                                          {MIB_TCP_STATE_SYN_RCVD, "syn_recv"},
                                                          {MIB_TCP_STATE_FIN_WAIT1, "fin_wait1"},
                                                          {MIB_TCP_STATE_FIN_WAIT2, "fin_wait2"},
                                                          {MIB_TCP_STATE_TIME_WAIT, "time_wait"},
                                                          {MIB_TCP_STATE_CLOSED, "close"},
                                                          {MIB_TCP_STATE_CLOSE_WAIT, "close_wait"},
                                                          {MIB_TCP_STATE_LAST_ACK, "last_ack"},
                                                          {MIB_TCP_STATE_LISTEN, "listening"},
                                                          {MIB_TCP_STATE_CLOSING, "closing"},
                                                          {MIB_TCP_STATE_DELETE_TCB, "delete_tcp"}};

static const std::map<pid_t, std::string> SYSTEM_PROCESSES = {
    {0, "System Idle Process"},
    {4, "System"},
};

struct PortTables
{
    std::unique_ptr<MIB_TCPTABLE_OWNER_PID[]> tcp;
    std::unique_ptr<MIB_TCP6TABLE_OWNER_PID[]> tcp6;
    std::unique_ptr<MIB_UDPTABLE_OWNER_PID[]> udp;
    std::unique_ptr<MIB_UDP6TABLE_OWNER_PID[]> udp6;
};

class WindowsPortWrapper final : public IPortWrapper
{
    const std::string m_protocol;
    const int32_t m_localPort;
    const std::string m_localIpAddress;
    const int32_t m_remotePort;
    const std::string m_remoteIpAddress;
    const uint32_t m_state;
    const uint32_t m_pid;
    const std::string m_processName;

    static std::string getIpV4Address(const DWORD addr)
    {
        in_addr ipaddress;
        ipaddress.S_un.S_addr = addr;
        return Utils::IAddressToString(AF_INET, ipaddress);
    }

    static std::string getProcessName(const std::map<pid_t, std::string> processDataList, const pid_t pid)
    {
        std::string retVal {EMPTY_VALUE};
        const auto itSystemProcess {SYSTEM_PROCESSES.find(pid)};

        if (SYSTEM_PROCESSES.end() != itSystemProcess)
        {
            retVal = itSystemProcess->second;
        }
        else
        {
            const auto itCurrentProcessList {processDataList.find(pid)};
            {
                if (processDataList.end() != itCurrentProcessList)
                {
                    retVal = itCurrentProcessList->second;
                }
            }
        }

        return retVal;
    }

    WindowsPortWrapper() = delete;

public:
    WindowsPortWrapper(const _MIB_TCPROW_OWNER_PID& data, const std::map<pid_t, std::string>& processDataList)
        : m_protocol {"tcp"}
        , m_localPort {static_cast<int32_t>(ntohs(static_cast<u_short>(data.dwLocalPort)))}
        , m_localIpAddress {getIpV4Address(data.dwLocalAddr)}
        , m_remotePort {static_cast<int32_t>(ntohs(static_cast<u_short>(data.dwRemotePort)))}
        , m_remoteIpAddress {getIpV4Address(data.dwRemoteAddr)}
        , m_state {data.dwState}
        , m_pid {data.dwOwningPid}
        , m_processName {getProcessName(processDataList, data.dwOwningPid)}
    {
    }

    WindowsPortWrapper(const _MIB_TCP6ROW_OWNER_PID& data, const std::map<pid_t, std::string>& processDataList)
        : m_protocol {"tcp6"}
        , m_localPort {static_cast<int32_t>(ntohs(static_cast<u_short>(data.dwLocalPort)))}
        , m_localIpAddress {Utils::getIpV6Address(data.ucLocalAddr)}
        , m_remotePort {static_cast<int32_t>(ntohs(static_cast<u_short>(data.dwRemotePort)))}
        , m_remoteIpAddress {Utils::getIpV6Address(data.ucRemoteAddr)}
        , m_state {data.dwState}
        , m_pid {data.dwOwningPid}
        , m_processName {getProcessName(processDataList, data.dwOwningPid)}
    {
    }

    WindowsPortWrapper(const _MIB_UDPROW_OWNER_PID& data, const std::map<pid_t, std::string>& processDataList)
        : m_protocol {"udp"}
        , m_localPort {static_cast<int32_t>(ntohs(static_cast<u_short>(data.dwLocalPort)))}
        , m_localIpAddress {getIpV4Address(data.dwLocalAddr)}
        , m_remotePort {0}
        , m_state {0}
        , m_pid {data.dwOwningPid}
        , m_processName {getProcessName(processDataList, data.dwOwningPid)}
    {
    }

    WindowsPortWrapper(const _MIB_UDP6ROW_OWNER_PID& data, const std::map<pid_t, std::string>& processDataList)
        : m_protocol("udp6")
        , m_localPort {static_cast<int32_t>(ntohs(static_cast<u_short>(data.dwLocalPort)))}
        , m_localIpAddress {Utils::getIpV6Address(data.ucLocalAddr)}
        , m_remotePort {0}
        , m_state {0}
        , m_pid {data.dwOwningPid}
        , m_processName {getProcessName(processDataList, data.dwOwningPid)}
    {
    }

    ~WindowsPortWrapper() = default;

    void protocol(nlohmann::json& port) const override
    {
        port["protocol"] = m_protocol;
    }

    void localIp(nlohmann::json& port) const override
    {
        port["local_ip"] = m_localIpAddress;
    }

    void localPort(nlohmann::json& port) const override
    {
        port["local_port"] = m_localPort;
    }

    void remoteIP(nlohmann::json& port) const override
    {
        port["remote_ip"] = m_remoteIpAddress;
    }

    void remotePort(nlohmann::json& port) const override
    {
        port["remote_port"] = m_remotePort;
    }

    void txQueue(nlohmann::json& port) const override
    {
        port["tx_queue"] = UNKNOWN_VALUE;
    }

    void rxQueue(nlohmann::json& port) const override
    {
        port["rx_queue"] = UNKNOWN_VALUE;
    }

    void inode(nlohmann::json& port) const override
    {
        port["inode"] = 0;
    }

    void state(nlohmann::json& port) const override
    {
        port["state"] = UNKNOWN_VALUE;
        const auto itState {STATE_TYPE.find(m_state)};

        if (STATE_TYPE.end() != itState)
        {
            port["state"] = itState->second;
        }
    }

    void pid(nlohmann::json& port) const override
    {
        port["pid"] = m_pid;
    }

    void processName(nlohmann::json& port) const override
    {
        port["process"] = Utils::stringAnsiToStringUTF8(m_processName);
    }
};

#endif //_PORT_WINDOWS_WRAPPER_H
