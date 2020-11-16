/*
 * Wazuh SysCollector
 * Copyright (C) 2015-2020, Wazuh Inc.
 * October 7, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */
#include <iostream>
#include "syscollectorImp.h"
#include "stringHelper.h"
#include "hashHelper.h"

constexpr auto PACKAGES_SQL_STATEMENT
{
    R"(CREATE TABLE packages(
    name TEXT,
    version TEXT,
    vendor TEXT,
    install_time TEXT,
    location TEXT,
    architecture TEXT,
    groups TEXT,
    description TEXT,
    size TEXT,
    priority TEXT,
    multiarch TEXT,
    source TEXT,
    checksum TEXT,
    PRIMARY KEY (name,version,architecture)) WITHOUT ROWID;)"
};
constexpr auto PROCESSES_SQL_STATEMENT
{
    R"(CREATE TABLE processes (
    pid BIGINT,
    name TEXT,
    state TEXT,
    ppid BIGINT,
    utime BIGINT,
    stime BIGINT,
    cmd TEXT,
    argvs TEXT,
    euser TEXT,
    ruser TEXT,
    suser TEXT,
    egroup TEXT,
    rgroup TEXT,
    sgroup TEXT,
    fgroup TEXT,
    priority BIGINT,
    nice BIGINT,
    size BIGINT,
    vm_size BIGINT,
    resident BIGINT,
    share BIGINT,
    start_time BIGINT,
    pgrp BIGINT,
    session BIGINT,
    nlwp BIGINT,
    tgid BIGINT,
    tty BIGINT,
    processor BIGINT,
    checksum TEXT,
    PRIMARY KEY (pid)) WITHOUT ROWID;)"
};
constexpr auto PORTS_SQL_STATEMENT
{
    R"(CREATE TABLE ports (
       protocol TEXT,
       local_ip TEXT,
       local_port BIGINT,
       remote_ip TEXT,
       remote_port BIGINT,
       tx_queue BIGINT,
       rx_queue BIGINT,
       inode BIGINT,
       state TEXT,
       pid BIGINT,
       process_name TEXT,
       checksum TEXT,
       PRIMARY KEY (inode, protocol, local_port)) WITHOUT ROWID;)"
};
constexpr auto NETIFACE_SQL_STATEMENT
{
    R"(CREATE TABLE network_iface (
       name TEXT,
       adapter TEXT,
       type TEXT,
       state TEXT,
       mtu TEXT,
       mac TEXT,
       tx_packets INTEGER,
       rx_packets INTEGER,
       tx_bytes INTEGER,
       rx_bytes INTEGER,
       tx_errors INTEGER,
       rx_errors INTEGER,
       tx_dropped INTEGER,
       rx_dropped INTEGER,
       checksum TEXT,
       PRIMARY KEY (name)) WITHOUT ROWID;)"
};
constexpr auto NETPROTO_SQL_STATEMENT
{
    R"(CREATE TABLE network_protocol (
       iface TEXT,
       type TEXT,
       gateway TEXT,
       dhcp TEXT NOT NULL CHECK (dhcp IN ('enabled', 'disabled', 'unknown', 'BOOTP')) DEFAULT 'unknown',
       metric TEXT,
       checksum TEXT,
       PRIMARY KEY (iface,type)) WITHOUT ROWID;)"
};
constexpr auto NETADDR_SQL_STATEMENT
{
    R"(CREATE TABLE network_address (
       iface TEXT,
       proto TEXT,
       address TEXT,
       netmask TEXT,
       broadcast TEXT,
       checksum TEXT,
       PRIMARY KEY (iface,proto,address)) WITHOUT ROWID;)"
};
constexpr auto OS_SQL_STATEMENT
{
    R"(CREATE TABLE os (
        hostname TEXT,
        architecture TEXT,
        os_name TEXT,
        os_version TEXT,
        os_codename TEXT,
        os_major TEXT,
        os_minor TEXT,
        os_build TEXT,
        os_platform TEXT,
        sysname TEXT,
        release TEXT,
        version TEXT,
        os_release TEXT,
        checksum TEXT,
        PRIMARY KEY (os_name)) WITHOUT ROWID;)"
};
constexpr auto HARDWARE_SQL_STATEMENT
{
    R"(CREATE TABLE hardware (
        board_serial TEXT,
        cpu_name TEXT,
        cpu_cores INTEGER CHECK (cpu_cores > 0),
        cpu_mhz BIGINT CHECK (cpu_mhz > 0),
        ram_total BIGINT CHECK (ram_total > 0),
        ram_free BIGINT CHECK (ram_free > 0),
        ram_usage INTEGER CHECK (ram_usage >= 0 AND ram_usage <= 100),
        checksum TEXT,
        PRIMARY KEY (board_serial)) WITHOUT ROWID;)"
};

static void updateAndNotifyChanges(const DBSYNC_HANDLE handle, const std::string& table, const nlohmann::json& values)
{
    const std::map<ReturnTypeCallback, std::string> operationsMap
    {
        // LCOV_EXCL_START
        {MODIFIED, "MODIFIED"},
        {DELETED , "DELETED "},
        {INSERTED, "INSERTED"},
        {MAX_ROWS, "MAX_ROWS"},
        {DB_ERROR, "DB_ERROR"},
        {SELECTED, "SELECTED"},
        // LCOV_EXCL_STOP
    };
    constexpr auto queueSize{4096};
    const auto callback
    {
        [&table, &operationsMap](ReturnTypeCallback result, const nlohmann::json& data)
        {
            if (data.is_array())
            {
                for (const auto& item : data)
                {
                    nlohmann::json msg;
                    msg["type"] = table;
                    msg["operation"] = operationsMap.at(result);
                    msg["data"] = item;
                    std::cout << msg.dump() << std::endl;
                }
            }
            else
            {
                // LCOV_EXCL_START
                nlohmann::json msg;
                msg["type"] = table;
                msg["operation"] = operationsMap.at(result);
                msg["data"] = data;
                std::cout << msg.dump() << std::endl;
                // LCOV_EXCL_STOP
            }
        }
    };
    DBSyncTxn txn
    {
        handle,
        nlohmann::json{table},
        0,
        queueSize,
        callback
    };
    nlohmann::json jsResult;
    nlohmann::json input;
    input["table"] = table;
    input["data"] = values;
    for (auto& item : input["data"])
    {
        const auto content{item.dump()};
        Utils::HashData hash;
        hash.update(content.c_str(), content.size());
        item["checksum"] = Utils::asciiToHex(hash.hash());
    }
    txn.syncTxnRow(input);
    txn.getDeletedRows(callback);
}

bool Syscollector::sleepFor()
{
    bool ret{false};
    std::unique_lock<std::mutex> lock{m_mutex};
    if (m_intervalUnit == "s")
    {
        ret = !m_cv.wait_for(lock, std::chrono::seconds{m_intervalValue}, [&](){return m_running;});
    }
    else if (m_intervalUnit == "m")
    {
        ret = !m_cv.wait_for(lock, std::chrono::minutes{m_intervalValue}, [&](){return m_running;});
    }
    else if (m_intervalUnit == "h")
    {
        ret = !m_cv.wait_for(lock, std::chrono::hours{m_intervalValue}, [&](){return m_running;});
    }
    else if (m_intervalUnit == "d")
    {
        const auto daysToHours{m_intervalValue * 24ul};
        ret = !m_cv.wait_for(lock, std::chrono::hours{daysToHours}, [&](){return m_running;});
    }
    else
    {
        ret = !m_cv.wait_for(lock, std::chrono::hours{1}, [&](){return m_running;});
    }
    return ret;
}

std::string Syscollector::getCreateStatement() const
{
    std::string ret;
    if (m_hardware)
    {
        ret += HARDWARE_SQL_STATEMENT;
    }
    if (m_os)
    {
        ret += OS_SQL_STATEMENT;
    }
    if (m_packages)
    {
        ret += PACKAGES_SQL_STATEMENT;
    }
    if (m_processes)
    {
        ret += PROCESSES_SQL_STATEMENT;
    }
    if (m_ports)
    {
        ret += PORTS_SQL_STATEMENT;
    }
    if (m_network)
    {
        ret += NETIFACE_SQL_STATEMENT;
        ret += NETPROTO_SQL_STATEMENT;
        ret += NETADDR_SQL_STATEMENT;
    }
    return ret;
}

Syscollector::Syscollector(const std::shared_ptr<ISysInfo>& spInfo,
                           const std::string& interval,
                           const bool scanOnStart,
                           const bool hardware,
                           const bool os,
                           const bool network,
                           const bool packages,
                           const bool ports,
                           const bool portsAll,
                           const bool processes,
                           const bool hotfixes)
: m_spInfo{spInfo}
, m_intervalUnit{interval.back()}
, m_intervalValue{std::stoull(interval)}
, m_scanOnStart{scanOnStart}
, m_hardware{hardware}
, m_os{os}
, m_network{network}
, m_packages{packages}
, m_ports{ports}
, m_portsAll{portsAll}
, m_processes{processes}
, m_hotfixes{hotfixes}
, m_running{false}
, m_dbSync{HostType::AGENT, DbEngineType::SQLITE3, "syscollector.db", getCreateStatement()}
, m_thread{std::bind(&Syscollector::syncThread, this)}
{
}

Syscollector::~Syscollector()
{
    std::unique_lock<std::mutex> lock{m_mutex};
    m_running = true;
    m_cv.notify_all();
    lock.unlock();
    if (m_thread.joinable())
    {
        m_thread.join();
    }
}
void Syscollector::scanHardware()
{
    if (m_hardware)
    {
        constexpr auto table{"hardware"};
        updateAndNotifyChanges(m_dbSync.handle(), table, nlohmann::json{m_spInfo->hardware()});
    }
}
void Syscollector::scanOs()
{
    if(m_os)
    {
        constexpr auto table{"os"};
        updateAndNotifyChanges(m_dbSync.handle(), table, nlohmann::json{m_spInfo->os()});
    }
}

void Syscollector::scanNetwork()
{
    if (m_network)
    {
        constexpr auto netIfaceTable    { "network_iface"    };
        constexpr auto netProtocolTable { "network_protocol" };
        constexpr auto netAddressTable  { "network_address"  };
        std::cout << "NETWORKS: " << m_spInfo->networks() << std::endl;
        const auto networks { m_spInfo->networks().at("iface") };
        nlohmann::json ifaceTableData{};
        nlohmann::json protoTableData{};
        nlohmann::json addressTableDataList{};

        for (const auto& item : networks.at(0))
        {
            // Split the resulting networks data into the specific DB tables

            // "network_iface" table data to update and notify
            ifaceTableData["name"]       = item.at("name");
            ifaceTableData["adapter"]    = item.at("adapter");
            ifaceTableData["type"]       = item.at("type");
            ifaceTableData["state"]      = item.at("state");
            ifaceTableData["mtu"]        = item.at("mtu");
            ifaceTableData["mac"]        = item.at("mac");
            ifaceTableData["tx_packets"] = item.at("tx_packets");
            ifaceTableData["rx_packets"] = item.at("rx_packets");
            ifaceTableData["tx_errors"]  = item.at("tx_errors");
            ifaceTableData["rx_errors"]  = item.at("rx_errors");
            ifaceTableData["tx_dropped"] = item.at("tx_dropped");
            ifaceTableData["rx_dropped"] = item.at("rx_dropped");

            // "network_protocol" table data to update and notify
            protoTableData["iface"]   = item.at("name");
            protoTableData["type"]    = item.at("type");
            protoTableData["gateway"] = item.at("gateway");

            if (item.find("IPv4") != item.end())
            {
                nlohmann::json addressTableData(item.at("IPv4"));
                protoTableData["dhcp"]    = addressTableData.at("dhcp");
                protoTableData["metric"]  = addressTableData.at("metric");

                // "network_address" table data to update and notify
                addressTableData["iface"]   = item.at("name");
                addressTableData["proto"]     = "IPv4";
                addressTableDataList.push_back(addressTableData);
            }

            if (item.find("IPv6") != item.end())
            {
                nlohmann::json addressTableData(item.at("IPv6"));
                protoTableData["dhcp"]    = addressTableData.at("dhcp");
                protoTableData["metric"]  = addressTableData.at("metric");

                // "network_address" table data to update and notify
                addressTableData["iface"] = item.at("name");
                addressTableData["proto"] = "IPv6";
                addressTableDataList.push_back(addressTableData);
            }
        }

        updateAndNotifyChanges(m_dbSync.handle(), netIfaceTable,    nlohmann::json{ifaceTableData});
        updateAndNotifyChanges(m_dbSync.handle(), netProtocolTable, nlohmann::json{protoTableData});
        updateAndNotifyChanges(m_dbSync.handle(), netAddressTable,  addressTableDataList);
    }
}

void Syscollector::scanPackages()
{
    if (m_packages)
    {
        constexpr auto table{"packages"};
        updateAndNotifyChanges(m_dbSync.handle(), table, m_spInfo->packages());
    }
}

void Syscollector::scanPorts()
{
    if (m_ports)
    {
        constexpr auto table{"ports"};
        constexpr auto PORT_LISTENING_STATE { "listening" };
        constexpr auto TCP_PROTOCOL { "tcp" };
        std::cout << "PORTS: " << m_spInfo->ports() << std::endl;        
        const auto data { m_spInfo->ports().at("ports") };
        nlohmann::json portsList{};
        for (const auto& item : data.at(0))
        {
            const auto isListeningState { item.at("state") == PORT_LISTENING_STATE };
            if(isListeningState)
            {
                // Only update and notify "Listening" state ports
                if (m_portsAll)
                {
                    // TCP and UDP ports
                    portsList.push_back(item);
                }
                else
                {
                    // Only TCP ports
                    const auto isTCPProto { item.at("protocol") == TCP_PROTOCOL };
                    if (isTCPProto)
                    {
                        portsList.push_back(item);
                    }
                }
            }
        }
        updateAndNotifyChanges(m_dbSync.handle(), table, portsList);
    }
}

void Syscollector::scanProcesses()
{
    if (m_processes)
    {
        constexpr auto table{"processes"};
        updateAndNotifyChanges(m_dbSync.handle(), table, m_spInfo->processes());
    }
}

void Syscollector::scan()
{
    scanHardware();
    scanOs();
    scanNetwork();
    scanPackages();
    scanPorts();
    scanProcesses();
}

void Syscollector::syncThread()
{
    if (m_scanOnStart)
    {
        scan();
    }
    while(sleepFor())
    {
        scan();
        //sync Rsync
    }
}
