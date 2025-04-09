#include "statelessEvent.hpp"

#include <commonDefs.h>
#include <config.h>
#include <hashHelper.hpp>
#include <inventory.hpp>
#include <iostream>
#include <nlohmann/json.hpp>
#include <stringHelper.hpp>
#include <timeHelper.hpp>

constexpr auto EMPTY_VALUE {""};

constexpr std::time_t INVENTORY_DEFAULT_INTERVAL {3600000};
constexpr size_t MAX_ID_SIZE = 512;

constexpr auto QUEUE_SIZE {4096};

static const std::map<ReturnTypeCallback, std::string> OPERATION_MAP {
    {MODIFIED, "update"},
    {DELETED, "delete"},
    {INSERTED, "create"},
    {MAX_ROWS, "max_rows"},
    {DB_ERROR, "db_error"},
    {SELECTED, "selected"},
};

constexpr auto SYSTEM_SQL_STATEMENT {
    R"(CREATE TABLE system (
    hostname TEXT,
    architecture TEXT,
    os_name TEXT,
    os_version TEXT,
    os_codename TEXT,
    os_build TEXT,
    os_platform TEXT,
    sysname TEXT,
    PRIMARY KEY (os_name)) WITHOUT ROWID;)"};

constexpr auto HARDWARE_SQL_STATEMENT {
    R"(CREATE TABLE hardware (
    board_serial TEXT,
    cpu_name TEXT,
    cpu_cores INTEGER,
    cpu_mhz INTEGER,
    ram_total INTEGER,
    ram_free INTEGER,
    ram_usage INTEGER,
    PRIMARY KEY (board_serial)) WITHOUT ROWID;)"};

constexpr auto HOTFIXES_SQL_STATEMENT {
    R"(CREATE TABLE hotfixes(
    hotfix TEXT,
    PRIMARY KEY (hotfix)) WITHOUT ROWID;)"};

constexpr auto PACKAGES_SQL_STATEMENT {
    R"(CREATE TABLE packages(
    name TEXT,
    version TEXT,
    install_time TEXT,
    location TEXT,
    architecture TEXT,
    description TEXT,
    size BIGINT,
    format TEXT,
    PRIMARY KEY (name,version,architecture,format,location)) WITHOUT ROWID;)"};

constexpr auto PROCESSES_SQL_STATEMENT {
    R"(CREATE TABLE processes (
    pid TEXT,
    name TEXT,
    ppid BIGINT,
    cmd TEXT,
    argvs TEXT,
    euser TEXT,
    ruser TEXT,
    suser TEXT,
    egroup TEXT,
    rgroup TEXT,
    sgroup TEXT,
    start_time BIGINT,
    tgid BIGINT,
    tty BIGINT,
    PRIMARY KEY (pid)) WITHOUT ROWID;)"};

constexpr auto PORTS_SQL_STATEMENT {
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
       PRIMARY KEY (inode,protocol,local_ip,local_port)) WITHOUT ROWID;)"};
static const std::vector<std::string> PORTS_ITEM_ID_FIELDS {"inode", "protocol", "local_ip", "local_port"};

constexpr auto NETWORKS_SQL_STATEMENT {
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
        dhcp TEXT,
        metric TEXT,
        address TEXT,
        netmask TEXT,
        broadcast TEXT,
        PRIMARY KEY (iface, adapter, iface_type, proto_type, address)
        ) WITHOUT ROWID;)"};

constexpr auto METADATA_SQL_STATEMENT {
    R"(CREATE TABLE metadata(
    key TEXT,
    value TEXT,
    PRIMARY KEY (key)) WITHOUT ROWID;)"};

constexpr auto NETWORKS_TABLE {"networks"};
constexpr auto PACKAGES_TABLE {"packages"};
constexpr auto HOTFIXES_TABLE {"hotfixes"};
constexpr auto PORTS_TABLE {"ports"};
constexpr auto PROCESSES_TABLE {"processes"};
constexpr auto SYSTEM_TABLE {"system"};
constexpr auto HARDWARE_TABLE {"hardware"};
constexpr auto MD_TABLE {"metadata"};

const std::unordered_map<std::string, std::string> TABLE_TO_KEY_MAP = {{NETWORKS_TABLE, "networks-first-scan"},
                                                                       {PACKAGES_TABLE, "packages-first-scan"},
                                                                       {HOTFIXES_TABLE, "hotfixes-first-scan"},
                                                                       {PORTS_TABLE, "ports-first-scan"},
                                                                       {PROCESSES_TABLE, "processes-first-scan"},
                                                                       {SYSTEM_TABLE, "system-first-scan"},
                                                                       {HARDWARE_TABLE, "hardware-first-scan"}};

static std::string GetItemId(const nlohmann::json& item, const std::vector<std::string>& idFields)
{
    Utils::HashData hash;

    for (const auto& field : idFields)
    {
        const auto& value {item.at(field)};

        if (!value.is_null())
        {
            if (value.is_string())
            {
                const auto& valueString {value.get<std::string>()};
                hash.update(valueString.c_str(), valueString.size());
            }
            else
            {
                const auto& valueNumber {value.get<unsigned long>()};
                const auto valueString {std::to_string(valueNumber)};
                hash.update(valueString.c_str(), valueString.size());
            }
        }
    }

    return Utils::asciiToHex(hash.hash());
}

static bool IsElementDuplicated(const nlohmann::json& input, const std::pair<std::string, std::string>& keyValue)
{
    const auto it {std::find_if(input.begin(),
                                input.end(),
                                [&keyValue](const auto& elem) { return elem.at(keyValue.first) == keyValue.second; })};
    return it != input.end();
}

nlohmann::json Inventory::EcsData(const nlohmann::json& data, const std::string& table, bool createFields)
{
    nlohmann::json ret;
    if (table == HARDWARE_TABLE)
    {
        ret = EcsHardwareData(data, createFields);
    }
    else if (table == SYSTEM_TABLE)
    {
        ret = EcsSystemData(data, createFields);
    }
    else if (table == PACKAGES_TABLE)
    {
        ret = EcsPackageData(data, createFields);
    }
    else if (table == PROCESSES_TABLE)
    {
        ret = EcsProcessesData(data, createFields);
    }
    else if (table == HOTFIXES_TABLE)
    {
        ret = EcsHotfixesData(data, createFields);
    }
    else if (table == PORTS_TABLE)
    {
        ret = EcsPortData(data, createFields);
    }
    else if (table == NETWORKS_TABLE)
    {
        ret = EcsNetworkData(data, createFields);
    }
    return ret;
}

std::string Inventory::GetPrimaryKeys([[maybe_unused]] const nlohmann::json& data, const std::string& table)
{
    std::string ret;
    if (table == HARDWARE_TABLE)
    {
        ret = data["board_serial"];
    }
    else if (table == SYSTEM_TABLE)
    {
        ret = data["os_name"];
    }
    else if (table == PACKAGES_TABLE)
    {
        ret = data["name"].get<std::string>() + ":" + data["version"].get<std::string>() + ":" +
              data["architecture"].get<std::string>() + ":" + data["format"].get<std::string>() + ":" +
              data["location"].get<std::string>();
    }
    else if (table == PROCESSES_TABLE)
    {
        ret = data["pid"];
    }
    else if (table == HOTFIXES_TABLE)
    {
        ret = data["hotfix"];
    }
    else if (table == PORTS_TABLE)
    {
        ret = std::to_string(data["inode"].get<int>()) + ":" + data["protocol"].get<std::string>() + ":" +
              data["local_ip"].get<std::string>() + ":" + std::to_string(data["local_port"].get<int>());
    }
    else if (table == NETWORKS_TABLE)
    {
        ret = data["iface"].get<std::string>() + ":" + data["adapter"].get<std::string>() + ":" +
              data["iface_type"].get<std::string>() + ":" + data["proto_type"].get<std::string>() + ":" +
              data["address"].get<std::string>();
    }
    return ret;
}

std::string Inventory::CalculateHashId(const nlohmann::json& data, const std::string& table)
{
    const std::string primaryKey = GetPrimaryKeys(data, table);
    const std::string baseId = AgentUUID() + ":" + primaryKey;

    Utils::HashData hash(Utils::HashType::Sha1);
    hash.update(baseId.c_str(), baseId.size());

    return Utils::asciiToHex(hash.hash());
}

void Inventory::NotifyChange(ReturnTypeCallback result,
                             const nlohmann::json& data,
                             const std::string& table,
                             const bool isFirstScan)
{
    if (DB_ERROR == result)
    {
        LogError("{}", data.dump());
        return;
    }

    if (!m_notify)
    {
        return;
    }

    if (data.is_array())
    {
        for (const auto& item : data)
        {
            ProcessEvent(result, item, table, isFirstScan);
        }
    }
    else
    {
        ProcessEvent(result, data, table, isFirstScan);
    }
}

void Inventory::ProcessEvent(ReturnTypeCallback result,
                             const nlohmann::json& item,
                             const std::string& table,
                             const bool isFirstScan)
{
    nlohmann::json msg = GenerateMessage(result, item, table);

    if (msg["metadata"]["id"].is_string() && msg["metadata"]["id"].get<std::string>().size() <= MAX_ID_SIZE)
    {
        NotifyEvent(result, msg, item, table, isFirstScan);
    }
    else
    {
        LogWarn("Event discarded for exceeding maximum size allowed in id field.");
        LogTrace("Event discarded: {}", msg.dump());
    }
}

nlohmann::json
Inventory::GenerateMessage(ReturnTypeCallback result, const nlohmann::json& item, const std::string& table)
{
    nlohmann::json msg {
        {"metadata", {{"collector", table}, {"operation", OPERATION_MAP.at(result)}, {"module", Name()}}}};

    msg["metadata"]["id"] = CalculateHashId(result == MODIFIED ? item["new"] : item, table);
    msg["data"] = EcsData(result == MODIFIED ? item["new"] : item, table);

    return msg;
}

void Inventory::NotifyEvent(ReturnTypeCallback result,
                            nlohmann::json& msg,
                            const nlohmann::json& item,
                            const std::string& table,
                            const bool isFirstScan)
{
    if (!isFirstScan)
    {
        const nlohmann::json oldData = (result == MODIFIED) ? EcsData(item["old"], table, false) : nlohmann::json {};

        nlohmann::json stateless = GenerateStatelessEvent(OPERATION_MAP.at(result), table, msg["data"]);
        nlohmann::json eventWithChanges = msg["data"];

        if (!oldData.empty())
        {
            stateless["event"]["changed_fields"] = AddPreviousFields(eventWithChanges, oldData);
        }

        stateless.update(eventWithChanges);
        msg["stateless"] = stateless;
    }

    msg["data"]["@timestamp"] = m_scanTime;

    const auto msgToSend = msg.dump();
    m_reportDiffFunction(msgToSend);
}

void Inventory::UpdateChanges(const std::string& table, const nlohmann::json& values, const bool isFirstScan)
{
    // NOLINTBEGIN(bugprone-exception-escape)
    const auto callback {[this, table, isFirstScan](ReturnTypeCallback result, const nlohmann::json& data)
                         {
                             NotifyChange(result, data, table, isFirstScan);
                         }};
    // NOLINTEND(bugprone-exception-escape)

    const std::unique_lock<std::mutex> lock {m_mutex};
    DBSyncTxn txn {m_spDBSync->handle(), nlohmann::json {table}, 0, QUEUE_SIZE, callback};
    nlohmann::json input;
    input["table"] = table;
    input["data"] = values;
    if (!isFirstScan)
    {
        input["options"]["return_old_data"] = true;
    }
    txn.syncTxnRow(input);
    txn.getDeletedRows(callback);
}

void Inventory::TryCatchTask(const std::function<void()>& task) const
{
    try
    {
        if (!m_stopping)
        {
            task();
        }
        else
        {
            LogTrace("No Scanning during stopping");
        }
    }
    catch (const std::exception& ex)
    {
        LogError("{}", std::string {ex.what()});
    }
}

Inventory::Inventory()
    : m_enabled {true}
    , m_dbFilePath {std::string(config::DEFAULT_DATA_PATH) + "/" + INVENTORY_DB_DISK_NAME}
    , m_normConfigPath {std::string(config::DEFAULT_DATA_PATH) + "/" + INVENTORY_NORM_CONFIG_NAME}
    , m_intervalValue {INVENTORY_DEFAULT_INTERVAL}
    , m_scanOnStart {true}
    , m_hardware {true}
    , m_system {true}
    , m_networks {true}
    , m_packages {true}
    , m_ports {true}
    , m_portsAll {true}
    , m_processes {true}
    , m_hotfixes {true}
    , m_stopping {true}
    , m_notify {true}
    , m_hardwareFirstScan {true}
    , m_systemFirstScan {true}
    , m_networksFirstScan {true}
    , m_packagesFirstScan {true}
    , m_portsFirstScan {true}
    , m_processesFirstScan {true}
    , m_hotfixesFirstScan {true}
{
}

std::string Inventory::GetCreateStatement() const
{
    std::string ret;

    ret += SYSTEM_SQL_STATEMENT;
    ret += HARDWARE_SQL_STATEMENT;
    ret += PACKAGES_SQL_STATEMENT;
    ret += HOTFIXES_SQL_STATEMENT;
    ret += PROCESSES_SQL_STATEMENT;
    ret += PORTS_SQL_STATEMENT;
    ret += NETWORKS_SQL_STATEMENT;
    ret += METADATA_SQL_STATEMENT;
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

    {
        const std::unique_lock<std::mutex> lock {m_mutex};
        m_stopping = false;
        m_spDBSync = std::make_unique<DBSync>(
            HostType::AGENT, DbEngineType::SQLITE3, dbPath, GetCreateStatement(), DbManagement::PERSISTENT);
        m_spNormalizer = std::make_unique<InvNormalizer>(normalizerConfigPath, normalizerType);
    }

    m_hardwareFirstScan = ReadMetadata(TABLE_TO_KEY_MAP.at(HARDWARE_TABLE)).empty() ? false : true;
    m_systemFirstScan = ReadMetadata(TABLE_TO_KEY_MAP.at(SYSTEM_TABLE)).empty() ? false : true;
    m_networksFirstScan = ReadMetadata(TABLE_TO_KEY_MAP.at(NETWORKS_TABLE)).empty() ? false : true;
    m_packagesFirstScan = ReadMetadata(TABLE_TO_KEY_MAP.at(PACKAGES_TABLE)).empty() ? false : true;
    m_portsFirstScan = ReadMetadata(TABLE_TO_KEY_MAP.at(PORTS_TABLE)).empty() ? false : true;
    m_processesFirstScan = ReadMetadata(TABLE_TO_KEY_MAP.at(PROCESSES_TABLE)).empty() ? false : true;
    m_hotfixesFirstScan = ReadMetadata(TABLE_TO_KEY_MAP.at(HOTFIXES_TABLE)).empty() ? false : true;

    if (m_hardwareFirstScan && !m_hardware)
    {
        DeleteMetadata(TABLE_TO_KEY_MAP.at(HARDWARE_TABLE));
        m_hardwareFirstScan = false;
    }

    if (m_systemFirstScan && !m_system)
    {
        DeleteMetadata(TABLE_TO_KEY_MAP.at(SYSTEM_TABLE));
        m_systemFirstScan = false;
    }

    if (m_networksFirstScan && !m_networks)
    {
        DeleteMetadata(TABLE_TO_KEY_MAP.at(NETWORKS_TABLE));
        m_networksFirstScan = false;
    }

    if (m_packagesFirstScan && !m_packages)
    {
        DeleteMetadata(TABLE_TO_KEY_MAP.at(PACKAGES_TABLE));
        m_packagesFirstScan = false;
    }

    if (m_portsFirstScan && !m_ports)
    {
        DeleteMetadata(TABLE_TO_KEY_MAP.at(PORTS_TABLE));
        m_portsFirstScan = false;
    }

    if (m_processesFirstScan && !m_processes)
    {
        DeleteMetadata(TABLE_TO_KEY_MAP.at(PROCESSES_TABLE));
        m_processesFirstScan = false;
    }

    if (m_hotfixesFirstScan && !m_hotfixes)
    {
        DeleteMetadata(TABLE_TO_KEY_MAP.at(HOTFIXES_TABLE));
        m_hotfixesFirstScan = false;
    }

    SyncLoop();
}

nlohmann::json Inventory::EcsHardwareData(const nlohmann::json& originalData, bool createFields)
{
    nlohmann::json ret;

    SetJsonField(ret, originalData, "/observer/serial_number", "board_serial", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/cpu/name", "cpu_name", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/cpu/cores", "cpu_cores", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/cpu/speed", "cpu_mhz", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/memory/total", "ram_total", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/memory/free", "ram_free", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/memory/used/percentage", "ram_usage", std::nullopt, createFields);

    return ret;
}

nlohmann::json Inventory::EcsSystemData(const nlohmann::json& originalData, bool createFields)
{
    nlohmann::json ret;

    SetJsonField(ret, originalData, "/host/architecture", "architecture", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/hostname", "hostname", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/os/kernel", "os_build", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/os/full", "os_codename", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/os/name", "os_name", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/os/platform", "os_platform", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/os/version", "os_version", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/os/type", "sysname", std::nullopt, createFields);

    return ret;
}

nlohmann::json Inventory::EcsPackageData(const nlohmann::json& originalData, bool createFields)
{
    nlohmann::json ret;

    SetJsonField(ret, originalData, "/package/architecture", "architecture", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/package/description", "description", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/package/installed", "install_time", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/package/name", "name", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/package/path", "location", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/package/size", "size", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/package/type", "format", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/package/version", "version", std::nullopt, createFields);

    return ret;
}

nlohmann::json Inventory::EcsProcessesData(const nlohmann::json& originalData, bool createFields)
{
    nlohmann::json ret;

    SetJsonField(ret, originalData, "/process/pid", "pid", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/process/name", "name", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/process/parent/pid", "ppid", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/process/command_line", "cmd", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/process/args", "argvs", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/process/user/id", "euser", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/process/real_user/id", "ruser", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/process/saved_user/id", "suser", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/process/group/id", "egroup", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/process/real_group/id", "rgroup", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/process/saved_group/id", "sgroup", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/process/start", "start_time", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/process/thread/id", "tgid", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/process/tty/char_device/major", "tty", std::nullopt, createFields);

    return ret;
}

nlohmann::json Inventory::EcsHotfixesData(const nlohmann::json& originalData, bool createFields)
{

    nlohmann::json ret;

    SetJsonField(ret, originalData, "/package/hotfix/name", "hotfix", std::nullopt, createFields);

    return ret;
}

nlohmann::json Inventory::EcsPortData(const nlohmann::json& originalData, bool createFields)
{
    nlohmann::json ret;

    SetJsonField(ret, originalData, "/network/protocol", "protocol", std::nullopt, createFields);
    SetJsonFieldArray(ret, originalData, "/source/ip", "local_ip", createFields);
    SetJsonField(ret, originalData, "/source/port", "local_port", std::nullopt, createFields);
    SetJsonFieldArray(ret, originalData, "/destination/ip", "remote_ip", createFields);
    SetJsonField(ret, originalData, "/destination/port", "remote_port", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/network/egress/queue", "tx_queue", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/network/ingress/queue", "rx_queue", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/file/inode", "inode", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/interface/state", "state", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/process/pid", "pid", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/process/name", "process", std::nullopt, createFields);

    return ret;
}

nlohmann::json Inventory::EcsNetworkData(const nlohmann::json& originalData, bool createFields)
{
    nlohmann::json ret;

    SetJsonFieldArray(ret, originalData, "/host/ip", "address", createFields);
    SetJsonField(ret, originalData, "/host/mac", "mac", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/network/egress/bytes", "tx_bytes", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/network/egress/packets", "tx_packets", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/network/ingress/bytes", "rx_bytes", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/network/ingress/packets", "rx_packets", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/network/egress/drops", "tx_dropped", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/network/egress/errors", "tx_errors", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/network/ingress/drops", "rx_dropped", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/host/network/ingress/errors", "rx_errors", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/interface/mtu", "mtu", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/interface/state", "state", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/interface/type", "iface_type", std::nullopt, createFields);
    SetJsonFieldArray(ret, originalData, "/network/netmask", "netmask", createFields);
    SetJsonFieldArray(ret, originalData, "/network/gateway", "gateway", createFields);
    SetJsonFieldArray(ret, originalData, "/network/broadcast", "broadcast", createFields);
    SetJsonField(ret, originalData, "/network/dhcp", "dhcp", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/network/type", "proto_type", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/network/metric", "metric", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/observer/ingress/interface/alias", "adapter", std::nullopt, createFields);
    SetJsonField(ret, originalData, "/observer/ingress/interface/name", "iface", std::nullopt, createFields);

    return ret;
}

void Inventory::ScanHardware()
{
    if (m_hardware)
    {
        LogTrace("Starting hardware scan");
        nlohmann::json hwData;
        hwData[0] = m_spInfo->hardware();
        UpdateChanges(HARDWARE_TABLE, hwData, !m_hardwareFirstScan);
        LogTrace("Ending hardware scan");

        if (!m_hardwareFirstScan && !m_stopping)
        {
            WriteMetadata(TABLE_TO_KEY_MAP.at(HARDWARE_TABLE), Utils::getCurrentISO8601());
            m_hardwareFirstScan = true;
        }
    }
}

void Inventory::ScanSystem()
{
    if (m_system)
    {
        LogTrace("Starting os scan");
        nlohmann::json SystemData;
        SystemData[0] = m_spInfo->os();
        UpdateChanges(SYSTEM_TABLE, SystemData, !m_systemFirstScan);
        LogTrace("Ending os scan");

        if (!m_systemFirstScan && !m_stopping)
        {
            WriteMetadata(TABLE_TO_KEY_MAP.at(SYSTEM_TABLE), Utils::getCurrentISO8601());
            m_systemFirstScan = true;
        }
    }
}

nlohmann::json Inventory::GetNetworkData()
{
    nlohmann::json ret;
    nlohmann::json networkTableData {};
    constexpr auto IPV4 {0};
    constexpr auto IPV6 {1};
    static const std::map<int, std::string> IP_TYPE {{IPV4, "ipv4"}, {IPV6, "ipv6"}};

    const auto& networks {m_spInfo->networks()};

    ret[NETWORKS_TABLE] = nlohmann::json::array();
    if (!networks.is_null())
    {
        const auto& itIface {networks.find("iface")};

        if (networks.end() != itIface)
        {
            for (const auto& item : itIface.value())
            {
                networkTableData["iface"] = item.at("name");
                networkTableData["adapter"] = item.at("adapter");
                networkTableData["iface_type"] = item.at("type");
                networkTableData["state"] = item.at("state");
                networkTableData["mtu"] = item.at("mtu");
                networkTableData["mac"] = item.at("mac");
                networkTableData["tx_packets"] = item.at("tx_packets");
                networkTableData["rx_packets"] = item.at("rx_packets");
                networkTableData["tx_errors"] = item.at("tx_errors");
                networkTableData["rx_errors"] = item.at("rx_errors");
                networkTableData["tx_bytes"] = item.at("tx_bytes");
                networkTableData["rx_bytes"] = item.at("rx_bytes");
                networkTableData["tx_dropped"] = item.at("tx_dropped");
                networkTableData["rx_dropped"] = item.at("rx_dropped");
                networkTableData["gateway"] = item.at("gateway");

                if (item.find("IPv4") != item.end())
                {
                    for (auto addressTableData : item.at("IPv4"))
                    {
                        nlohmann::json networkAddressData {};
                        networkAddressData["proto_type"] = IP_TYPE.at(IPV4);
                        networkAddressData["address"] = addressTableData.at("address");
                        networkAddressData["broadcast"] = addressTableData.at("broadcast");
                        networkAddressData["dhcp"] = addressTableData.at("dhcp");
                        networkAddressData["metric"] = addressTableData.at("metric");
                        networkAddressData["netmask"] = addressTableData.at("netmask");
                        networkTableData.update(networkAddressData);

                        ret[NETWORKS_TABLE].push_back(networkTableData);
                    }
                }

                if (item.find("IPv6") != item.end())
                {
                    for (auto addressTableData : item.at("IPv6"))
                    {
                        nlohmann::json networkAddressData {};
                        networkAddressData["proto_type"] = IP_TYPE.at(IPV6);
                        networkAddressData["address"] = addressTableData.at("address");
                        networkAddressData["broadcast"] = addressTableData.at("broadcast");
                        networkAddressData["dhcp"] = addressTableData.at("dhcp");
                        networkAddressData["metric"] = addressTableData.at("metric");
                        networkAddressData["netmask"] = addressTableData.at("netmask");
                        networkTableData.update(networkAddressData);

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
        LogTrace("Starting network scan");
        const auto networkData(GetNetworkData());

        if (!networkData.is_null())
        {
            const auto itNet {networkData.find(NETWORKS_TABLE)};

            if (itNet != networkData.end())
            {
                UpdateChanges(NETWORKS_TABLE, itNet.value(), !m_networksFirstScan);
            }
        }

        if (!m_networksFirstScan && !m_stopping)
        {
            WriteMetadata(TABLE_TO_KEY_MAP.at(NETWORKS_TABLE), Utils::getCurrentISO8601());
            m_networksFirstScan = true;
        }

        LogTrace("Ending network scan");
    }
}

void Inventory::ScanPackages()
{
    if (m_packages)
    {
        LogTrace("Starting packages scan");
        const auto callback {[this](ReturnTypeCallback result, const nlohmann::json& data)
                             {
                                 NotifyChange(result, data, PACKAGES_TABLE, !m_packagesFirstScan);
                             }};

        const std::unique_lock<std::mutex> lock {m_mutex};
        DBSyncTxn txn {m_spDBSync->handle(), nlohmann::json {PACKAGES_TABLE}, 0, QUEUE_SIZE, callback};
        m_spInfo->packages(
            [this, &txn](nlohmann::json& rawData)
            {
                if (m_stopping)
                {
                    return;
                }

                nlohmann::json input;

                input["table"] = PACKAGES_TABLE;
                m_spNormalizer->Normalize("packages", rawData);
                m_spNormalizer->RemoveExcluded("packages", rawData);

                if (!rawData.empty())
                {
                    input["data"] = nlohmann::json::array({rawData});
                    if (m_packagesFirstScan)
                    {
                        input["options"]["return_old_data"] = true;
                    }
                    txn.syncTxnRow(input);
                }
            });
        txn.getDeletedRows(callback);

        if (!m_packagesFirstScan && !m_stopping)
        {
            WriteMetadata(TABLE_TO_KEY_MAP.at(PACKAGES_TABLE), Utils::getCurrentISO8601());
            m_packagesFirstScan = true;
        }

        LogTrace("Ending packages scan");
    }
}

void Inventory::ScanHotfixes()
{
    if (m_hotfixes)
    {
        LogTrace("Starting hotfixes scan");
        auto hotfixes = m_spInfo->hotfixes();

        if (!hotfixes.is_null())
        {
            UpdateChanges(HOTFIXES_TABLE, hotfixes, !m_hotfixesFirstScan);
        }

        if (!m_hotfixesFirstScan && !m_stopping)
        {
            WriteMetadata(TABLE_TO_KEY_MAP.at(HOTFIXES_TABLE), Utils::getCurrentISO8601());
            m_hotfixesFirstScan = true;
        }

        LogTrace("Ending hotfixes scan");
    }
}

nlohmann::json Inventory::GetPortsData()
{
    nlohmann::json ret;
    constexpr auto PORT_LISTENING_STATE {"listening"};
    constexpr auto TCP_PROTOCOL {"tcp"};
    constexpr auto UDP_PROTOCOL {"udp"};
    auto data(m_spInfo->ports());

    if (!data.is_null())
    {
        for (auto& item : data)
        {
            const auto protocol {item.at("protocol").get_ref<const std::string&>()};

            if (Utils::startsWith(protocol, TCP_PROTOCOL))
            {
                // All ports.
                if (m_portsAll)
                {
                    const auto& itemId {GetItemId(item, PORTS_ITEM_ID_FIELDS)};

                    if (!IsElementDuplicated(ret, std::make_pair("item_id", itemId)))
                    {
                        item["item_id"] = itemId;
                        ret.push_back(item);
                    }
                }
                else
                {
                    // Only listening ports.
                    const auto isListeningState {item.at("state") == PORT_LISTENING_STATE};

                    if (isListeningState)
                    {
                        const auto& itemId {GetItemId(item, PORTS_ITEM_ID_FIELDS)};

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
                const auto& itemId {GetItemId(item, PORTS_ITEM_ID_FIELDS)};

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
        LogTrace("Starting ports scan");
        const auto& portsData {GetPortsData()};
        UpdateChanges(PORTS_TABLE, portsData, !m_portsFirstScan);
        LogTrace("Ending ports scan");

        if (!m_portsFirstScan && !m_stopping)
        {
            WriteMetadata(TABLE_TO_KEY_MAP.at(PORTS_TABLE), Utils::getCurrentISO8601());
            m_portsFirstScan = true;
        }
    }
}

void Inventory::ScanProcesses()
{
    if (m_processes)
    {
        LogTrace("Starting processes scan");
        const auto callback {[this](ReturnTypeCallback result, const nlohmann::json& data)
                             {
                                 NotifyChange(result, data, PROCESSES_TABLE, !m_processesFirstScan);
                             }};
        const std::unique_lock<std::mutex> lock {m_mutex};
        DBSyncTxn txn {m_spDBSync->handle(), nlohmann::json {PROCESSES_TABLE}, 0, QUEUE_SIZE, callback};
        m_spInfo->processes(std::function<void(nlohmann::json&)>(
            [this, &txn](nlohmann::json& rawData)
            {
                if (m_stopping)
                {
                    return;
                }

                nlohmann::json input;
                input["table"] = PROCESSES_TABLE;
                input["data"] = nlohmann::json::array({rawData});

                if (m_processesFirstScan)
                {
                    input["options"]["return_old_data"] = true;
                }

                txn.syncTxnRow(input);
            }));
        txn.getDeletedRows(callback);

        if (!m_processesFirstScan && !m_stopping)
        {
            WriteMetadata(TABLE_TO_KEY_MAP.at(PROCESSES_TABLE), Utils::getCurrentISO8601());
            m_processesFirstScan = true;
        }

        LogTrace("Ending processes scan");
    }
}

void Inventory::Scan()
{
    LogInfo("Starting evaluation.");
    m_scanTime = Utils::getCurrentISO8601();

    TryCatchTask([&]() { ScanHardware(); });
    TryCatchTask([&]() { ScanSystem(); });
    TryCatchTask([&]() { ScanPackages(); });
    TryCatchTask([&]() { ScanProcesses(); });
    TryCatchTask([&]() { ScanHotfixes(); });
    TryCatchTask([&]() { ScanPorts(); });
    TryCatchTask([&]() { ScanNetwork(); });

    m_notify = true;
    LogInfo("Evaluation finished.");
}

void Inventory::SyncLoop()
{
    LogInfo("Module started.");

    if (m_scanOnStart && !m_stopping)
    {
        Scan();
    }

    while (!m_stopping)
    {
        {
            std::unique_lock<std::mutex> lock {m_mutex};
            m_cv.wait_for(lock, std::chrono::milliseconds {m_intervalValue}, [&]() { return m_stopping.load(); });
        }
        Scan();
    }
    const std::unique_lock<std::mutex> lock {m_mutex};
    m_spDBSync.reset(nullptr);
}

void Inventory::WriteMetadata(const std::string& key, const std::string& value)
{
    auto insertQuery {InsertQuery::builder().table(MD_TABLE).data({{"key", key}, {"value", value}}).build()};
    m_spDBSync->insertData(insertQuery.query());
}

std::string Inventory::ReadMetadata(const std::string& key)
{
    std::string result;
    const std::string filter = "WHERE key = '" + key + "'";
    auto selectQuery = SelectQuery::builder().table(MD_TABLE).columnList({"key", "value"}).rowFilter(filter).build();

    auto callback = [&result](ReturnTypeCallback returnTypeCallback, const nlohmann::json& resultData)
    {
        (void)returnTypeCallback;
        if (resultData.is_object() && resultData.contains("key") && resultData.contains("value"))
        {
            result = resultData["value"];
        }
    };

    m_spDBSync->selectRows(selectQuery.query(), callback);

    return result;
}

void Inventory::DeleteMetadata(const std::string& key)
{
    auto deleteQuery {DeleteQuery::builder().table(MD_TABLE).data({{"key", key}}).rowFilter("").build()};
    m_spDBSync->deleteRows(deleteQuery.query());
}

void Inventory::CleanMetadata()
{
    try
    {
        {
            const std::unique_lock<std::mutex> lock {m_mutex};
            m_spDBSync = std::make_unique<DBSync>(
                HostType::AGENT, DbEngineType::SQLITE3, m_dbFilePath, GetCreateStatement(), DbManagement::PERSISTENT);
            for (const auto& key : TABLE_TO_KEY_MAP)
            {
                if (!ReadMetadata(key.second).empty())
                {
                    DeleteMetadata(key.second);
                }
            }
            m_spDBSync.reset();
        }
    }
    catch (const std::exception& ex)
    {
        LogError("{}", ex.what());
    }
}

nlohmann::json Inventory::AddPreviousFields(nlohmann::json& current, const nlohmann::json& previous)
{
    using JsonPair = std::pair<nlohmann::json*, const nlohmann::json*>;
    using PathPair = std::pair<std::string, JsonPair>;

    std::stack<PathPair> stack;
    nlohmann::json modifiedKeys = nlohmann::json::array();

    stack.emplace("", JsonPair(&current, &previous));

    while (!stack.empty())
    {
        auto [path, pair] = stack.top();
        auto [curr, prev] = pair;
        stack.pop();

        for (auto& [key, value] : prev->items())
        {

            std::string currentPath = path;
            if (!path.empty())
            {
                currentPath.append(".").append(key);
            }
            else
            {
                currentPath = key;
            }

            if (curr->contains(key))
            {
                if ((*curr)[key].is_object() && value.is_object())
                {
                    stack.emplace(currentPath, JsonPair(&((*curr)[key]), &value));
                }
                else if ((*curr)[key] != value)
                {
                    modifiedKeys.push_back(currentPath);
                    (*curr)["previous"][key] = value;
                }
            }
        }
    }
    return modifiedKeys;
}

nlohmann::json
Inventory::GenerateStatelessEvent(const std::string& operation, const std::string& type, const nlohmann::json& data)
{
    auto event = CreateStatelessEvent(type, operation, m_scanTime, data);
    return event ? event->generate() : nlohmann::json {};
}

void Inventory::SetJsonField(nlohmann::json& target,
                             const nlohmann::json& source,
                             const std::string& keyPath,
                             const std::string& jsonKey,
                             const std::optional<std::string>& defaultValue,
                             bool createFields)
{
    if (createFields || source.contains(jsonKey))
    {
        const nlohmann::json::json_pointer pointer(keyPath);
        if (source.contains(jsonKey) && source[jsonKey] != EMPTY_VALUE)
        {
            target[pointer] = source[jsonKey];
        }
        else if (defaultValue.has_value())
        {
            target[pointer] = *defaultValue;
        }
        else
        {
            target[pointer] = nullptr;
        }
    }
}

void Inventory::SetJsonFieldArray(nlohmann::json& target,
                                  const nlohmann::json& source,
                                  const std::string& destPath,
                                  const std::string& sourceKey,
                                  bool createFields)
{
    if (createFields || source.contains(sourceKey))
    {
        const nlohmann::json::json_pointer destPointer(destPath);
        target[destPointer] = nullptr;

        if (source.contains(sourceKey) && !source[sourceKey].is_null() && source[sourceKey] != EMPTY_VALUE)
        {
            const auto& value = source[sourceKey];
            target[destPointer] = nlohmann::json::array();
            target[destPointer].push_back(value);
        }
    }
}
