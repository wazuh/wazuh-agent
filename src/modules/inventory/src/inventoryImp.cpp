#include <config.h>
#include <defs.h>
#include <inventory.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <stringHelper.h>
#include <hashHelper.h>
#include <timeHelper.h>
#include <boost/beast/core/detail/base64.hpp>

constexpr std::time_t INVENTORY_DEFAULT_INTERVAL { 3600000 };

constexpr auto QUEUE_SIZE
{
    4096
};

static const std::map<ReturnTypeCallback, std::string> OPERATION_MAP
{
    // LCOV_EXCL_START
    {MODIFIED, "modified"},
    {DELETED, "deleted"},
    {INSERTED, "create"},
    {MAX_ROWS, "max_rows"},
    {DB_ERROR, "db_error"},
    {SELECTED, "selected"},
    // LCOV_EXCL_STOP
};

constexpr auto OS_SQL_STATEMENT
{
    R"(CREATE TABLE system (
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
    R"(CREATE TABLE hardware (
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
       PRIMARY KEY (inode,protocol,local_ip,local_port)) WITHOUT ROWID;)"
};
static const std::vector<std::string> PORTS_ITEM_ID_FIELDS{"inode", "protocol", "local_ip", "local_port"};

constexpr auto NETWORK_SQL_STATEMENT
{
    R"(CREATE TABLE networks (
        iface TEXT,
        adapter TEXT,
        iface_type TEXT,
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
        proto_type TEXT,
        gateway TEXT,
        dhcp TEXT NOT NULL CHECK (dhcp IN ('enabled', 'disabled', 'unknown', 'BOOTP')) DEFAULT 'unknown',
        metric TEXT,
        address TEXT,
        netmask TEXT,
        broadcast TEXT,
        network_item_id TEXT,
        network_checksum TEXT,
        PRIMARY KEY (iface, adapter, iface_type, proto_type, address)
        ) WITHOUT ROWID;)"
};

static const std::vector<std::string> NETWORK_ITEM_ID_FIELDS{"iface", "adapter", "iface_type", "proto_type", "address"};

constexpr auto NETWORKS_TABLE     { "networks"  };
constexpr auto PACKAGES_TABLE     { "packages"  };
constexpr auto HOTFIXES_TABLE     { "hotfixes"  };
constexpr auto PORTS_TABLE        { "ports"     };
constexpr auto PROCESSES_TABLE    { "processes" };
constexpr auto OS_TABLE           { "system"    };
constexpr auto HW_TABLE           { "hardware"  };


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
    else if (table == OS_TABLE)
    {
        ret = EcsSystemData(data);
    }
    else if (table == PACKAGES_TABLE)
    {
        ret = EcsPackageData(data);
    }
    else if (table == PROCESSES_TABLE)
    {
        ret = EcsProcessesData(data);
    }
    else if(table == HOTFIXES_TABLE)
    {
        ret = EcsHotfixesData(data);
    }
    else if (table == PORTS_TABLE)
    {
        ret = EcsPortData(data);
    }
    else if (table == NETWORKS_TABLE)
    {
        ret = EcsNetworkData(data);
    }
    return ret;
}

std::string Inventory::GetPrimaryKeys([[maybe_unused]] const nlohmann::json& data, const std::string& table)
{
    std::string ret;
    if (table == HW_TABLE)
    {
        ret = data["observer"]["serial_number"];
    }
    else if (table == OS_TABLE)
    {
        ret = data["host"]["os"]["name"];
    }
    else if (table == PACKAGES_TABLE)
    {
        ret = data["package"]["name"].get<std::string>() + ":" + data["package"]["version"].get<std::string>() + ":" + data["package"]["architecture"].get<std::string>() + ":" + data["package"]["type"].get<std::string>() + ":" + data["package"]["path"].get<std::string>();
    }
    else if (table == PROCESSES_TABLE)
    {
        ret = data["process"]["pid"];
    }
    else if(table == HOTFIXES_TABLE)
    {
        ret = data["package"]["hotfix"]["name"];
    }
    else if (table == PORTS_TABLE)
    {
        ret = std::to_string(data["file"]["inode"].get<int>()) + ":" + data["network"]["protocol"].get<std::string>() + ":" + data["source"]["ip"][0].get<std::string>() + ":" + std::to_string(data["source"]["port"].get<int>());
    }
    else if (table == NETWORKS_TABLE)
    {
        ret = data["observer"]["ingress"]["interface"]["name"].get<std::string>() + ":" + data["observer"]["ingress"]["interface"]["alias"].get<std::string>() + ":" + data["network"]["type"].get<std::string>() + ":" + data["network"]["protocol"].get<std::string>() + ":" + data["host"]["ip"][0].get<std::string>();
    }
    return ret;
}

std::string Inventory::CalculateBase64Id(const nlohmann::json& data, const std::string& table)
{
    std::string primaryKey = GetPrimaryKeys(data, table);
    std::string baseId = Name() + ":" + table + ":" + primaryKey;
    std::string idBase64;
    idBase64.resize(boost::beast::detail::base64::encoded_size(baseId.size()));
    boost::beast::detail::base64::encode(&idBase64[0], baseId.c_str(), baseId.size());
    return idBase64;
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
                msg["id"] = CalculateBase64Id(msg["data"], table);
                msg["data"]["@timestamp"] = m_scanTime;
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
            msg["id"] = CalculateBase64Id(msg["data"], table);
            msg["data"]["@timestamp"] = m_scanTime;
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
    , m_dbFilePath { std::string(config::DEFAULT_DATA_PATH) + "/" + INVENTORY_DB_DISK_NAME }
    , m_intervalValue { INVENTORY_DEFAULT_INTERVAL }
    , m_scanOnStart { true }
    , m_hardware { true }
    , m_system { true }
    , m_networks { true }
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
    ret += NETWORK_SQL_STATEMENT;
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

nlohmann::json Inventory::EcsSystemData(const nlohmann::json& originalData)
{
    nlohmann::json ret;

    ret["host"]["architecture"] = originalData.contains("architecture") ? originalData["architecture"] : "";
    ret["host"]["hostname"] = originalData.contains("hostname") ? originalData["hostname"] : "";
    ret["host"]["os"]["kernel"] = originalData.contains("os_build") ? originalData["os_build"] : "";
    ret["host"]["os"]["full"] = originalData.contains("os_codename") ? originalData["os_codename"] : "";
    ret["host"]["os"]["name"] = originalData.contains("os_name") ? originalData["os_name"] : "";
    ret["host"]["os"]["platform"] = originalData.contains("os_platform") ? originalData["os_platform"] : "";
    ret["host"]["os"]["version"]= originalData.contains("os_version") ? originalData["os_version"] : "";
    ret["host"]["os"]["type"]= originalData.contains("sysname") ? originalData["sysname"] : "";

    return ret;
}

nlohmann::json Inventory::EcsPackageData(const nlohmann::json& originalData)
{
    nlohmann::json ret;

    ret["package"]["architecture"] = originalData.contains("architecture") ? originalData["architecture"] : "";
    ret["package"]["description"] = originalData.contains("description") ? originalData["description"] : "";
    ret["package"]["installed"] = originalData.contains("install_time") ? originalData["install_time"] : "";
    ret["package"]["name"] = originalData.contains("name") ? originalData["name"] : "";
    ret["package"]["path"] = originalData.contains("location") ? originalData["location"] : "";
    ret["package"]["size"] = originalData.contains("size") ? originalData["size"] : nlohmann::json(0);
    ret["package"]["type"] = originalData.contains("format") ? originalData["format"] : "";
    ret["package"]["version"] = originalData.contains("version") ? originalData["version"] : "";

    return ret;
}

nlohmann::json Inventory::EcsProcessesData(const nlohmann::json& originalData)
{
    nlohmann::json ret;

    ret["process"]["pid"] = originalData.contains("pid") ? originalData["pid"] : nlohmann::json(0);
    ret["process"]["name"] = originalData.contains("name") ? originalData["name"] : "";
    ret["process"]["parent"]["pid"] = originalData.contains("ppid") ? originalData["ppid"] : nlohmann::json(0);
    ret["process"]["command_line"] = originalData.contains("cmd") ? originalData["cmd"] : "";
    ret["process"]["args"] = originalData.contains("argvs") ? originalData["argvs"] : "";
    ret["process"]["user"]["id"] = originalData.contains("euser") ? originalData["euser"] : "";
    ret["process"]["real_user"]["id"]= originalData.contains("ruser") ? originalData["ruser"] : "";
    ret["process"]["saved_user"]["id"]= originalData.contains("suser") ? originalData["suser"] : "";
    ret["process"]["group"]["id"]= originalData.contains("egroup") ? originalData["egroup"] : "";
    ret["process"]["real_group"]["id"]= originalData.contains("rgroup") ? originalData["rgroup"] : "";
    ret["process"]["saved_group"]["id"]= originalData.contains("sgroup") ? originalData["sgroup"] : "";
    ret["process"]["start"]= originalData.contains("start_time") ? originalData["start_time"] : "";
    ret["process"]["thread"]["id"]= originalData.contains("tgid") ? originalData["tgid"] : "";
    ret["process"]["tty"]= originalData.contains("tty") ? originalData["tty"] : "";

    return ret;
}

nlohmann::json Inventory::EcsHotfixesData(const nlohmann::json& originalData){

    nlohmann::json ret;

    ret["package"]["hotfix"]["name"] = originalData.contains("hotfix") ? originalData["hotfix"] : "";

    return ret;
}

nlohmann::json Inventory::EcsPortData(const nlohmann::json& originalData)
{
    nlohmann::json ret;

    ret["network"]["protocol"] = originalData.contains("protocol") ? originalData["protocol"] : "";
    ret["source"]["ip"] = nlohmann::json::array();
    ret["source"]["ip"].push_back(originalData.contains("local_ip") ? originalData["local_ip"] : "");
    ret["source"]["port"] = originalData.contains("local_port") ? originalData["local_port"] : nlohmann::json(0);
    ret["destination"]["ip"] = nlohmann::json::array();
    ret["destination"]["ip"].push_back(originalData.contains("remote_ip") ? originalData["remote_ip"] : "");
    ret["destination"]["port"] = originalData.contains("remote_port") ? originalData["remote_port"] : nlohmann::json(0);
    ret["host"]["network"]["egress"]["queue"] = originalData.contains("tx_queue") ? originalData["tx_queue"] : nlohmann::json(0);
    ret["host"]["network"]["ingress"]["queue"] = originalData.contains("rx_queue") ? originalData["rx_queue"] : nlohmann::json(0);
    ret["file"]["inode"] = originalData.contains("inode") ? originalData["inode"] : nlohmann::json(0);
    ret["interface"]["state"] = originalData.contains("state") ? originalData["state"] : "";
    ret["process"]["pid"] = originalData.contains("pid") ? originalData["pid"] : nlohmann::json(0);
    ret["process"]["name"] = originalData.contains("process") ? originalData["process"] : "";
    ret["device"]["id"] = originalData.contains("item_id") ? originalData["item_id"] : "";

    return ret;
}

nlohmann::json Inventory::EcsNetworkData(const nlohmann::json& originalData)
{
    nlohmann::json ret;

    ret["host"]["ip"] = nlohmann::json::array();
    ret["host"]["ip"].push_back(originalData.contains("address") ? originalData["address"] : "");
    ret["host"]["mac"] = originalData.contains("mac") ? originalData["mac"] : "";
    ret["host"]["network"]["egress"]["bytes"] = originalData.contains("tx_bytes") ? originalData["tx_bytes"] : nlohmann::json(0);
    ret["host"]["network"]["egress"]["packets"] = originalData.contains("tx_packets") ? originalData["tx_packets"] : nlohmann::json(0);
    ret["host"]["network"]["ingress"]["bytes"] = originalData.contains("rx_bytes") ? originalData["rx_bytes"] : nlohmann::json(0);
    ret["host"]["network"]["ingress"]["packets"] = originalData.contains("rx_packets") ? originalData["rx_packets"] : nlohmann::json(0);
    ret["host"]["network"]["egress"]["drops"] = originalData.contains("rx_dropped") ? originalData["rx_dropped"] : nlohmann::json(0);
    ret["host"]["network"]["egress"]["errors"] = originalData.contains("tx_errors") ? originalData["tx_errors"] : nlohmann::json(0);
    ret["host"]["network"]["ingress"]["drops"] = originalData.contains("tx_dropped") ? originalData["tx_dropped"] : nlohmann::json(0);
    ret["host"]["network"]["ingress"]["errors"] = originalData.contains("rx_errors") ? originalData["rx_errors"] : nlohmann::json(0);

    ret["interface"]["mtu"] = originalData.contains("mtu") ? originalData["mtu"] : nlohmann::json(0);
    ret["interface"]["state"] = originalData.contains("state") ? originalData["state"] : "";
    ret["interface"]["type"] = originalData.contains("iface_type") ? originalData["iface_type"] : "";

    ret["network"]["netmask"] = nlohmann::json::array();
    ret["network"]["netmask"].push_back(originalData.contains("netmask") ? originalData["netmask"] : "");
    ret["network"]["gateway"] = nlohmann::json::array();
    ret["network"]["gateway"].push_back(originalData.contains("gateway") ? originalData["gateway"] : "");
    ret["network"]["broadcast"] = nlohmann::json::array();
    ret["network"]["broadcast"].push_back(originalData.contains("broadcast") ? originalData["broadcast"] : "");
    ret["network"]["dhcp"] = originalData.contains("dhcp") ? originalData["dhcp"] : "";
    ret["network"]["type"] = originalData.contains("proto_type") ? originalData["proto_type"] : "";
    ret["network"]["metric"] = originalData.contains("metric") ? originalData["metric"] : nlohmann::json(0);
    /* TODO this field should include http or https, it's related to an application not to a interface */
    ret["network"]["protocol"] = "";

    ret["observer"]["ingress"]["interface"]["alias"] = originalData.contains("adapter") ? originalData["adapter"] : "";
    ret["observer"]["ingress"]["interface"]["name"] = originalData.contains("iface") ? originalData["iface"] : "";

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
    return ret;
}

void Inventory::ScanOs()
{
    if (m_system)
    {
        LogTrace( "Starting os scan");
        const auto& osData{GetOSData()};
        UpdateChanges(OS_TABLE, osData);
        LogTrace( "Ending os scan");
    }
}

nlohmann::json Inventory::GetNetworkData()
{
    nlohmann::json ret ;
    nlohmann::json networkTableData {};
    constexpr auto IPV4 { 0 };
    constexpr auto IPV6 { 1 };
    static const std::map<int, std::string> IP_TYPE
    {
        { IPV4, "ipv4" },
        { IPV6, "ipv6" }
    };

    const auto& networks { m_spInfo->networks() };

    ret[NETWORKS_TABLE] = nlohmann::json::array();
    if (!networks.is_null())
    {
        const auto& itIface { networks.find("iface") };

        if (networks.end() != itIface)
        {
            for (const auto& item : itIface.value())
            {
                networkTableData["iface"]      = item.at("name");
                networkTableData["adapter"]    = item.at("adapter");
                networkTableData["iface_type"] = item.at("type");
                networkTableData["state"]      = item.at("state");
                networkTableData["mtu"]        = item.at("mtu");
                networkTableData["mac"]        = item.at("mac");
                networkTableData["tx_packets"] = item.at("tx_packets");
                networkTableData["rx_packets"] = item.at("rx_packets");
                networkTableData["tx_errors"]  = item.at("tx_errors");
                networkTableData["rx_errors"]  = item.at("rx_errors");
                networkTableData["tx_bytes"]   = item.at("tx_bytes");
                networkTableData["rx_bytes"]   = item.at("rx_bytes");
                networkTableData["tx_dropped"] = item.at("tx_dropped");
                networkTableData["rx_dropped"] = item.at("rx_dropped");
                networkTableData["gateway"] = item.at("gateway");

                if (item.find("IPv4") != item.end())
                {
                    for (auto addressTableData : item.at("IPv4"))
                    {
                        nlohmann::json networkAddressData {};
                        networkAddressData["proto_type"]    = IP_TYPE.at(IPV4);
                        networkAddressData["address"] = addressTableData.at("address");
                        networkAddressData["broadcast"] = addressTableData.at("broadcast");
                        networkAddressData["dhcp"]    = addressTableData.at("dhcp");
                        networkAddressData["metric"]  = addressTableData.at("metric");
                        networkAddressData["netmask"] = addressTableData.at("netmask");
                        networkTableData.update(networkAddressData);

                        networkTableData["network_item_id"] = GetItemId(networkTableData, NETWORK_ITEM_ID_FIELDS);

                        ret[NETWORKS_TABLE].push_back(networkTableData);

                    }
                }

                if (item.find("IPv6") != item.end())
                {
                    for (auto addressTableData : item.at("IPv6"))
                    {
                        nlohmann::json networkAddressData {};
                        networkAddressData["proto_type"]  = IP_TYPE.at(IPV6);
                        networkAddressData["address"] = addressTableData.at("address");
                        networkAddressData["broadcast"] = addressTableData.at("broadcast");
                        networkAddressData["dhcp"]    = addressTableData.at("dhcp");
                        networkAddressData["metric"]  = addressTableData.at("metric");
                        networkAddressData["netmask"] = addressTableData.at("netmask");
                        networkTableData.update(networkAddressData);

                        networkTableData["network_item_id"] = GetItemId(networkTableData, NETWORK_ITEM_ID_FIELDS);

                        ret[NETWORKS_TABLE].push_back(networkTableData);
                    }
                }
            }
        }
    }

    return ret;
}

void Inventory::ScanNetwork()
{
    if (m_networks)
    {
        LogTrace( "Starting network scan");
        const auto networkData(GetNetworkData());

        if (!networkData.is_null())
        {
            const auto itNet { networkData.find(NETWORKS_TABLE) };

            if (itNet != networkData.end())
            {
                UpdateChanges(NETWORKS_TABLE, itNet.value());
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
    TryCatchTask([&]() { ScanOs(); });
    TryCatchTask([&]() { ScanPackages(); });
    TryCatchTask([&]() { ScanProcesses(); });
    TryCatchTask([&]() { ScanHotfixes(); });
    TryCatchTask([&]() { ScanPorts(); });
    TryCatchTask([&]() { ScanNetwork(); });

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

    while (!m_cv.wait_for(lock, std::chrono::milliseconds{m_intervalValue}, [&]()
{
    return m_stopping;
}))
    {
        Scan();
    }
    m_spDBSync.reset(nullptr);
}
