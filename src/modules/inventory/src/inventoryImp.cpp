#include <inventory.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <stringHelper.h>
#include <hashHelper.h>
#include <timeHelper.h>

constexpr std::chrono::seconds INVENTORY_DEFAULT_INTERVAL { 3600 };

constexpr auto QUEUE_SIZE
{
    4096
};

static const std::map<ReturnTypeCallback, std::string> OPERATION_MAP
{
    // LCOV_EXCL_START
    {MODIFIED, "MODIFIED"},
    {DELETED, "DELETED"},
    {INSERTED, "INSERTED"},
    {MAX_ROWS, "MAX_ROWS"},
    {DB_ERROR, "DB_ERROR"},
    {SELECTED, "SELECTED"},
    // LCOV_EXCL_STOP
};

constexpr auto OS_SQL_STATEMENT
{
    R"(CREATE TABLE osinfo (
    hostname TEXT,
    architecture TEXT,
    os_name TEXT,
    os_version TEXT,
    os_codename TEXT,
    os_major TEXT,
    os_minor TEXT,
    os_patch TEXT,
    os_build TEXT,
    os_platform TEXT,
    sysname TEXT,
    release TEXT,
    version TEXT,
    os_release TEXT,
    os_display_version TEXT,
    checksum TEXT,
    PRIMARY KEY (os_name)) WITHOUT ROWID;)"
};

constexpr auto HW_SQL_STATEMENT
{
    R"(CREATE TABLE hwinfo (
    board_serial TEXT,
    cpu_name TEXT,
    cpu_cores INTEGER,
    cpu_mhz DOUBLE,
    ram_total INTEGER,
    ram_free INTEGER,
    ram_usage INTEGER,
    checksum TEXT,
    PRIMARY KEY (board_serial)) WITHOUT ROWID;)"
};

constexpr auto HOTFIXES_SQL_STATEMENT
{
    R"(CREATE TABLE hotfixes(
    hotfix TEXT,
    checksum TEXT,
    PRIMARY KEY (hotfix)) WITHOUT ROWID;)"
};

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
    size INTEGER,
    priority TEXT,
    multiarch TEXT,
    source TEXT,
    format TEXT,
    checksum TEXT,
    item_id TEXT,
    PRIMARY KEY (name,version,architecture,format,location)) WITHOUT ROWID;)"
};
static const std::vector<std::string> PACKAGES_ITEM_ID_FIELDS{"name", "version", "architecture", "format", "location"};

constexpr auto PROCESSES_SQL_STATEMENT
{
    R"(CREATE TABLE processes (
    pid TEXT,
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
       process TEXT,
       checksum TEXT,
       item_id TEXT,
       PRIMARY KEY (inode, protocol, local_ip, local_port)) WITHOUT ROWID;)"
};
static const std::vector<std::string> PORTS_ITEM_ID_FIELDS{"inode", "protocol", "local_ip", "local_port"};

constexpr auto NETIFACE_SQL_STATEMENT
{
    R"(CREATE TABLE network_iface (
       name TEXT,
       adapter TEXT,
       type TEXT,
       state TEXT,
       mtu BIGINT,
       mac TEXT,
       tx_packets INTEGER,
       rx_packets INTEGER,
       tx_bytes BIGINT,
       rx_bytes BIGINT,
       tx_errors INTEGER,
       rx_errors INTEGER,
       tx_dropped INTEGER,
       rx_dropped INTEGER,
       checksum TEXT,
       item_id TEXT,
       PRIMARY KEY (name,adapter,type)) WITHOUT ROWID;)"
};
static const std::vector<std::string> NETIFACE_ITEM_ID_FIELDS{"name", "adapter", "type"};

constexpr auto NETPROTO_SQL_STATEMENT
{
    R"(CREATE TABLE network_protocol (
       iface TEXT,
       type TEXT,
       gateway TEXT,
       dhcp TEXT NOT NULL CHECK (dhcp IN ('enabled', 'disabled', 'unknown', 'BOOTP')) DEFAULT 'unknown',
       metric TEXT,
       checksum TEXT,
       item_id TEXT,
       PRIMARY KEY (iface,type)) WITHOUT ROWID;)"
};
static const std::vector<std::string> NETPROTO_ITEM_ID_FIELDS{"iface", "type"};

constexpr auto NETADDR_SQL_STATEMENT
{
    R"(CREATE TABLE network_address (
       iface TEXT,
       proto INTEGER,
       address TEXT,
       netmask TEXT,
       broadcast TEXT,
       checksum TEXT,
       item_id TEXT,
       PRIMARY KEY (iface,proto,address)) WITHOUT ROWID;)"
};
static const std::vector<std::string> NETADDRESS_ITEM_ID_FIELDS{"iface", "proto", "address"};

constexpr auto NET_IFACE_TABLE    { "network_iface"    };
constexpr auto NET_PROTOCOL_TABLE { "network_protocol" };
constexpr auto NET_ADDRESS_TABLE  { "network_address"  };
constexpr auto PACKAGES_TABLE     { "packages"         };
constexpr auto HOTFIXES_TABLE     { "hotfixes"         };
constexpr auto PORTS_TABLE        { "ports"            };
constexpr auto PROCESSES_TABLE    { "processes"        };
constexpr auto OS_TABLE           { "osinfo"           };
constexpr auto HW_TABLE           { "hwinfo"           };


static std::string GetItemId(const nlohmann::json& item, const std::vector<std::string>& idFields)
{
    Utils::HashData hash;

    for (const auto& field : idFields)
    {
        const auto& value{item.at(field)};

        if (value.is_string())
        {
            const auto& valueString{value.get<std::string>()};
            hash.update(valueString.c_str(), valueString.size());
        }
        else
        {
            const auto& valueNumber{value.get<unsigned long>()};
            const auto valueString{std::to_string(valueNumber)};
            hash.update(valueString.c_str(), valueString.size());
        }
    }

    return Utils::asciiToHex(hash.hash());
}

static std::string GetItemChecksum(const nlohmann::json& item)
{
    const auto content{item.dump()};
    Utils::HashData hash;
    hash.update(content.c_str(), content.size());
    return Utils::asciiToHex(hash.hash());
}

static void RemoveKeysWithEmptyValue(nlohmann::json& input)
{
    for (auto& data : input)
    {
        for (auto it = data.begin(); it != data.end(); )
        {
            if (it.value().type() == nlohmann::detail::value_t::string &&
                    it.value().get_ref<const std::string&>().empty())
            {
                it = data.erase(it);
            }
            else
            {
                ++it;
            }
        }
    }
}

static bool IsElementDuplicated(const nlohmann::json& input, const std::pair<std::string, std::string>& keyValue)
{
    const auto it
    {
        std::find_if (input.begin(), input.end(), [&keyValue](const auto & elem)
        {
            return elem.at(keyValue.first) == keyValue.second;
        })
    };
    return it != input.end();
}

nlohmann::json Inventory::EcsData(const nlohmann::json& data, const std::string& table)
{
    nlohmann::json ret;
    if(table == HW_TABLE)
    {
        ret = EcsHardwareData(data);
    }
    return ret;
}

void Inventory::NotifyChange(ReturnTypeCallback result, const nlohmann::json& data, const std::string& table)
{
    if (DB_ERROR == result)
    {
        LogErrorInventory(data.dump());
    }
    else if (m_notify && !m_stopping)
    {
        if (data.is_array())
        {
            for (const auto& item : data)
            {
                nlohmann::json msg;
                msg["type"] = table;
                msg["operation"] = OPERATION_MAP.at(result);
                msg["data"] = EcsData(item, table);
                msg["data"]["scan_time"] = m_scanTime;
                // TO DO: this is necesary for ECS?
                //RemoveKeysWithEmptyValue(msg["data"]);
                const auto msgToSend{msg.dump()};
                m_reportDiffFunction(msgToSend);
            }
        }
        else
        {
            // LCOV_EXCL_START
            nlohmann::json msg;
            msg["type"] = table;
            msg["operation"] = OPERATION_MAP.at(result);
            msg["data"] = EcsData(data, table);
            msg["data"]["scan_time"] = m_scanTime;
            RemoveKeysWithEmptyValue(msg["data"]);
            const auto msgToSend{msg.dump()};
            m_reportDiffFunction(msgToSend);
            // LCOV_EXCL_STOP
        }
    }
}

void Inventory::UpdateChanges(const std::string& table,
                                 const nlohmann::json& values)
{
    const auto callback
    {
        [this, table](ReturnTypeCallback result, const nlohmann::json & data)
        {
            NotifyChange(result, data, table);
        }
    };
    DBSyncTxn txn
    {
        m_spDBSync->handle(),
        nlohmann::json{table},
        0,
        QUEUE_SIZE,
        callback
    };
    nlohmann::json input;
    input["table"] = table;
    input["data"] = values;
    txn.syncTxnRow(input);
    txn.getDeletedRows(callback);
}

void Inventory::TryCatchTask(const std::function<void()>& task) const
{
    try
    {
        if (!m_stopping)
        {
            task();  // Ejecuta la tarea
        }
    }
    catch (const std::exception& ex)
    {
        LogErrorInventory(std::string{ex.what()});
    }
}

Inventory::Inventory()
    : m_enabled { true }
    , m_intervalValue { INVENTORY_DEFAULT_INTERVAL }
    , m_scanOnStart { true }
    , m_hardware { true }
    , m_os { true }
    , m_network { true }
    , m_packages { true }
    , m_ports { true }
    , m_portsAll { true }
    , m_processes { true }
    , m_hotfixes { true }
    , m_stopping { true }
    , m_notify { true }
{}

std::string Inventory::GetCreateStatement() const
{
    std::string ret;

    ret += OS_SQL_STATEMENT;
    ret += HW_SQL_STATEMENT;
    ret += PACKAGES_SQL_STATEMENT;
    ret += HOTFIXES_SQL_STATEMENT;
    ret += PROCESSES_SQL_STATEMENT;
    ret += PORTS_SQL_STATEMENT;
    ret += NETIFACE_SQL_STATEMENT;
    ret += NETPROTO_SQL_STATEMENT;
    ret += NETADDR_SQL_STATEMENT;
    return ret;
}

void Inventory::Init(const std::shared_ptr<ISysInfo>& spInfo,
                        const std::function<void(const std::string&)>& reportDiffFunction,
                        const std::string& dbPath,
                        const std::string& normalizerConfigPath,
                        const std::string& normalizerType)
{
    m_spInfo = spInfo;
    m_reportDiffFunction = reportDiffFunction;

    std::unique_lock<std::mutex> lock{m_mutex};
    m_stopping = false;
    m_spDBSync = std::make_unique<DBSync>(HostType::AGENT,
                                            DbEngineType::SQLITE3,
                                            dbPath,
                                            GetCreateStatement(),
                                            DbManagement::PERSISTENT);
    m_spNormalizer = std::make_unique<InvNormalizer>(normalizerConfigPath, normalizerType);
    SyncLoop(lock);
}

void Inventory::Destroy()
{
    std::unique_lock<std::mutex> lock{m_mutex};
    m_stopping = true;
    m_cv.notify_all();
    lock.unlock();
}


nlohmann::json Inventory::EcsHardwareData(const nlohmann::json& originalData)
{
    nlohmann::json ret;

    ret["observer"]["serial_number"] = originalData.contains("board_serial") ? originalData["board_serial"] : "";
    ret["host"]["cpu"]["name"] = originalData.contains("cpu_name") ? originalData["cpu_name"] : "";
    ret["host"]["cpu"]["cores"] = originalData.contains("cpu_cores") ? originalData["cpu_cores"] : nlohmann::json(0);
    ret["host"]["cpu"]["speed"] = originalData.contains("cpu_mhz") ? originalData["cpu_mhz"] : nlohmann::json(0);
    ret["host"]["memory"]["total"] = originalData.contains("ram_total") ? originalData["ram_total"] : nlohmann::json(0);
    ret["host"]["memory"]["free"] = originalData.contains("ram_free") ? originalData["ram_free"] : nlohmann::json(0);
    ret["host"]["memory"]["used"]["percentage"] = originalData.contains("ram_usage") ? originalData["ram_usage"] : nlohmann::json(0);

    return ret;
}


nlohmann::json Inventory::GetHardwareData()
{
    nlohmann::json ret;
    ret[0] = m_spInfo->hardware();
    return ret;
}

void Inventory::ScanHardware()
{
    if (m_hardware)
    {
        LogTrace( "Starting hardware scan");
        const auto& hwData{GetHardwareData()};
        UpdateChanges(HW_TABLE, hwData);
        LogTrace( "Ending hardware scan");
    }
}

nlohmann::json Inventory::GetOSData()
{
    nlohmann::json ret;
    ret[0] = m_spInfo->os();
    ret[0]["checksum"] = std::to_string(std::chrono::system_clock::now().time_since_epoch().count());
    return ret;
}

void Inventory::ScanOs()
{
    if (m_os)
    {
        LogTrace( "Starting os scan");
        const auto& osData{GetOSData()};
        UpdateChanges(OS_TABLE, osData);
        LogTrace( "Ending os scan");
    }
}

nlohmann::json Inventory::GetNetworkData()
{
    nlohmann::json ret;
    const auto& networks { m_spInfo->networks() };
    nlohmann::json ifaceTableDataList {};
    nlohmann::json protoTableDataList {};
    nlohmann::json addressTableDataList {};
    constexpr auto IPV4 { 0 };
    constexpr auto IPV6 { 1 };
    static const std::map<int, std::string> IP_TYPE
    {
        { IPV4, "ipv4" },
        { IPV6, "ipv6" }
    };

    if (!networks.is_null())
    {
        const auto& itIface { networks.find("iface") };

        if (networks.end() != itIface)
        {
            for (const auto& item : itIface.value())
            {
                // Split the resulting networks data into the specific DB tables
                // "dbsync_network_iface" table data to update and notify
                nlohmann::json ifaceTableData {};
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
                ifaceTableData["tx_bytes"]   = item.at("tx_bytes");
                ifaceTableData["rx_bytes"]   = item.at("rx_bytes");
                ifaceTableData["tx_dropped"] = item.at("tx_dropped");
                ifaceTableData["rx_dropped"] = item.at("rx_dropped");
                ifaceTableData["checksum"]   = GetItemChecksum(ifaceTableData);
                ifaceTableData["item_id"]    = GetItemId(ifaceTableData, NETIFACE_ITEM_ID_FIELDS);
                ifaceTableDataList.push_back(std::move(ifaceTableData));

                if (item.find("IPv4") != item.end())
                {
                    // "dbsync_network_protocol" table data to update and notify
                    nlohmann::json protoTableData {};
                    protoTableData["iface"]   = item.at("name");
                    protoTableData["gateway"] = item.at("gateway");
                    protoTableData["type"]    = IP_TYPE.at(IPV4);
                    protoTableData["dhcp"]    = item.at("IPv4").begin()->at("dhcp");
                    protoTableData["metric"]  = item.at("IPv4").begin()->at("metric");
                    protoTableData["checksum"]  = GetItemChecksum(protoTableData);
                    protoTableData["item_id"]   = GetItemId(protoTableData, NETPROTO_ITEM_ID_FIELDS);
                    protoTableDataList.push_back(std::move(protoTableData));

                    for (auto addressTableData : item.at("IPv4"))
                    {
                        // "dbsync_network_address" table data to update and notify
                        addressTableData["iface"]     = item.at("name");
                        addressTableData["proto"]     = IPV4;
                        addressTableData["checksum"]  = GetItemChecksum(addressTableData);
                        addressTableData["item_id"]   = GetItemId(addressTableData, NETADDRESS_ITEM_ID_FIELDS);
                        // Remove unwanted fields for dbsync_network_address table
                        addressTableData.erase("dhcp");
                        addressTableData.erase("metric");

                        addressTableDataList.push_back(std::move(addressTableData));
                    }
                }

                if (item.find("IPv6") != item.end())
                {
                    // "dbsync_network_protocol" table data to update and notify
                    nlohmann::json protoTableData {};
                    protoTableData["iface"]   = item.at("name");
                    protoTableData["gateway"] = item.at("gateway");
                    protoTableData["type"]    = IP_TYPE.at(IPV6);
                    protoTableData["dhcp"]    = item.at("IPv6").begin()->at("dhcp");
                    protoTableData["metric"]  = item.at("IPv6").begin()->at("metric");
                    protoTableData["checksum"]  = GetItemChecksum(protoTableData);
                    protoTableData["item_id"]   = GetItemId(protoTableData, NETPROTO_ITEM_ID_FIELDS);
                    protoTableDataList.push_back(std::move(protoTableData));

                    for (auto addressTableData : item.at("IPv6"))
                    {
                        // "dbsync_network_address" table data to update and notify
                        addressTableData["iface"]     = item.at("name");
                        addressTableData["proto"]     = IPV6;
                        addressTableData["checksum"]  = GetItemChecksum(addressTableData);
                        addressTableData["item_id"]   = GetItemId(addressTableData, NETADDRESS_ITEM_ID_FIELDS);
                        // Remove unwanted fields for dbsync_network_address table
                        addressTableData.erase("dhcp");
                        addressTableData.erase("metric");

                        addressTableDataList.push_back(std::move(addressTableData));
                    }
                }
            }

            ret[NET_IFACE_TABLE] = std::move(ifaceTableDataList);
            ret[NET_PROTOCOL_TABLE] = std::move(protoTableDataList);
            ret[NET_ADDRESS_TABLE] = std::move(addressTableDataList);
        }
    }

    return ret;
}

void Inventory::ScanNetwork()
{
    if (m_network)
    {
        LogTrace( "Starting network scan");
        const auto networkData(GetNetworkData());

        if (!networkData.is_null())
        {
            const auto itIface { networkData.find(NET_IFACE_TABLE) };

            if (itIface != networkData.end())
            {
                UpdateChanges(NET_IFACE_TABLE, itIface.value());
            }

            const auto itProtocol { networkData.find(NET_PROTOCOL_TABLE) };

            if (itProtocol != networkData.end())
            {
                UpdateChanges(NET_PROTOCOL_TABLE, itProtocol.value());
            }

            const auto itAddress { networkData.find(NET_ADDRESS_TABLE) };

            if (itAddress != networkData.end())
            {
                UpdateChanges(NET_ADDRESS_TABLE, itAddress.value());
            }
        }

        LogTrace( "Ending network scan");
    }
}

void Inventory::ScanPackages()
{
    if (m_packages)
    {
        LogTrace( "Starting packages scan");
        const auto callback
        {
            [this](ReturnTypeCallback result, const nlohmann::json & data)
            {
                NotifyChange(result, data, PACKAGES_TABLE);
            }
        };
        DBSyncTxn txn
        {
            m_spDBSync->handle(),
            nlohmann::json{PACKAGES_TABLE},
            0,
            QUEUE_SIZE,
            callback
        };
        m_spInfo->packages([this, &txn](nlohmann::json & rawData)
        {
            nlohmann::json input;

            rawData["checksum"] = GetItemChecksum(rawData);
            rawData["item_id"] = GetItemId(rawData, PACKAGES_ITEM_ID_FIELDS);

            input["table"] = PACKAGES_TABLE;
            m_spNormalizer->Normalize("packages", rawData);
            m_spNormalizer->RemoveExcluded("packages", rawData);

            if (!rawData.empty())
            {
                input["data"] = nlohmann::json::array( { rawData } );
                txn.syncTxnRow(input);
            }
        });
        txn.getDeletedRows(callback);

        LogTrace( "Ending packages scan");
    }
}

void Inventory::ScanHotfixes()
{
    if (m_hotfixes)
    {
        LogTrace( "Starting hotfixes scan");
        auto hotfixes = m_spInfo->hotfixes();

        if (!hotfixes.is_null())
        {
            for (auto& hotfix : hotfixes)
            {
                hotfix["checksum"] = GetItemChecksum(hotfix);
            }

            UpdateChanges(HOTFIXES_TABLE, hotfixes);
        }

        LogTrace( "Ending hotfixes scan");
    }
}

nlohmann::json Inventory::GetPortsData()
{
    nlohmann::json ret;
    constexpr auto PORT_LISTENING_STATE { "listening" };
    constexpr auto TCP_PROTOCOL { "tcp" };
    constexpr auto UDP_PROTOCOL { "udp" };
    auto data(m_spInfo->ports());

    if (!data.is_null())
    {
        for (auto& item : data)
        {
            const auto protocol { item.at("protocol").get_ref<const std::string&>() };

            if (Utils::startsWith(protocol, TCP_PROTOCOL))
            {
                // All ports.
                if (m_portsAll)
                {
                    const auto& itemId { GetItemId(item, PORTS_ITEM_ID_FIELDS) };

                    if (!IsElementDuplicated(ret, std::make_pair("item_id", itemId)))
                    {
                        item["checksum"] = GetItemChecksum(item);
                        item["item_id"] = itemId;
                        ret.push_back(item);
                    }
                }
                else
                {
                    // Only listening ports.
                    const auto isListeningState { item.at("state") == PORT_LISTENING_STATE };

                    if (isListeningState)
                    {
                        const auto& itemId { GetItemId(item, PORTS_ITEM_ID_FIELDS) };

                        if (!IsElementDuplicated(ret, std::make_pair("item_id", itemId)))
                        {
                            item["checksum"] = GetItemChecksum(item);
                            item["item_id"] = itemId;
                            ret.push_back(item);
                        }
                    }
                }
            }
            else if (Utils::startsWith(protocol, UDP_PROTOCOL))
            {
                const auto& itemId { GetItemId(item, PORTS_ITEM_ID_FIELDS) };

                if (!IsElementDuplicated(ret, std::make_pair("item_id", itemId)))
                {
                    item["checksum"] = GetItemChecksum(item);
                    item["item_id"] = itemId;
                    ret.push_back(item);
                }
            }
        }
    }

    return ret;
}

void Inventory::ScanPorts()
{
    if (m_ports)
    {
        LogTrace( "Starting ports scan");
        const auto& portsData { GetPortsData() };
        UpdateChanges(PORTS_TABLE, portsData);
        LogTrace( "Ending ports scan");
    }
}

void Inventory::ScanProcesses()
{
    if (m_processes)
    {
        LogTrace( "Starting processes scan");
        const auto callback
        {
            [this](ReturnTypeCallback result, const nlohmann::json & data)
            {
                NotifyChange(result, data, PROCESSES_TABLE);
            }
        };
        DBSyncTxn txn
        {
            m_spDBSync->handle(),
            nlohmann::json{PROCESSES_TABLE},
            0,
            QUEUE_SIZE,
            callback
        };
        m_spInfo->processes([&txn](nlohmann::json & rawData)
        {
            nlohmann::json input;

            rawData["checksum"] = GetItemChecksum(rawData);

            input["table"] = PROCESSES_TABLE;
            input["data"] = nlohmann::json::array( { rawData } );

            txn.syncTxnRow(input);
        });
        txn.getDeletedRows(callback);

        LogTrace( "Ending processes scan");
    }
}

void Inventory::Scan()
{
    LogInfo("Starting evaluation.");
    m_scanTime = Utils::getCurrentTimestamp();

    TryCatchTask([&]() { ScanHardware(); });
    // TO DO: enable each scan once the ECS translation is done
    //TryCatchTask([&]() { ScanOs(); });
    //TryCatchTask([&]() { ScanNetwork(); });
    //TryCatchTask([&]() { ScanPackages(); });
    //TryCatchTask([&]() { ScanHotfixes(); });
    //TryCatchTask([&]() { ScanPorts(); });
    //TryCatchTask([&]() { ScanProcesses(); });
    m_notify = true;
    LogInfo("Evaluation finished.");
}

void Inventory::SyncLoop(std::unique_lock<std::mutex>& lock)
{
    LogInfo("Module started.");

    if (m_scanOnStart)
    {
        Scan();
    }

    while (!m_cv.wait_for(lock, std::chrono::seconds{m_intervalValue}, [&]()
{
    return m_stopping;
}))
    {
        Scan();
    }
    m_spDBSync.reset(nullptr);
}

