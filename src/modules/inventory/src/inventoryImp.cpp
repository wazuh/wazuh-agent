#include <config.h>
#include <defs.h>
#include <sharedDefs.h>
#include <inventory.hpp>
#include <nlohmann/json.hpp>
#include <iostream>
#include <stringHelper.h>
#include <hashHelper.h>
#include <timeHelper.h>

constexpr std::time_t INVENTORY_DEFAULT_INTERVAL { 3600000 };
constexpr size_t MAX_ID_SIZE = 512;

constexpr auto QUEUE_SIZE
{
    4096
};

static const std::map<ReturnTypeCallback, std::string> OPERATION_MAP
{
    // LCOV_EXCL_START
    {MODIFIED, "update"},
    {DELETED, "delete"},
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
    PRIMARY KEY (os_name)) WITHOUT ROWID;)"
};

constexpr auto HW_SQL_STATEMENT
{
    R"(CREATE TABLE hardware (
    board_serial TEXT,
    cpu_name TEXT,
    cpu_cores INTEGER,
    cpu_mhz INTEGER,
    ram_total INTEGER,
    ram_free INTEGER,
    ram_usage INTEGER,
    PRIMARY KEY (board_serial)) WITHOUT ROWID;)"
};

constexpr auto HOTFIXES_SQL_STATEMENT
{
    R"(CREATE TABLE hotfixes(
    hotfix TEXT,
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
    PRIMARY KEY (name,version,architecture,format,location)) WITHOUT ROWID;)"
};

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
       PRIMARY KEY (inode,protocol,local_ip,local_port)) WITHOUT ROWID;)"
};
static const std::vector<std::string> PORTS_ITEM_ID_FIELDS{"inode", "protocol", "local_ip", "local_port"};

constexpr auto NETWORKS_SQL_STATEMENT
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
        dhcp TEXT,
        metric TEXT,
        address TEXT,
        netmask TEXT,
        broadcast TEXT,
        PRIMARY KEY (iface, adapter, iface_type, proto_type, address)
        ) WITHOUT ROWID;)"
};

constexpr auto METADATA_SQL_STATEMENT
{
    R"(CREATE TABLE metadata(
    key TEXT,
    value TEXT,
    PRIMARY KEY (key)) WITHOUT ROWID;)"
};

constexpr auto NETWORKS_TABLE     { "networks"  };
constexpr auto PACKAGES_TABLE     { "packages"  };
constexpr auto HOTFIXES_TABLE     { "hotfixes"  };
constexpr auto PORTS_TABLE        { "ports"     };
constexpr auto PROCESSES_TABLE    { "processes" };
constexpr auto OS_TABLE           { "system"    };
constexpr auto HW_TABLE           { "hardware"  };
constexpr auto MD_TABLE           { "metadata"  };

const std::unordered_map<std::string, std::string> TABLE_TO_KEY_MAP = {
    {NETWORKS_TABLE,    "networks-first-scan"},
    {PACKAGES_TABLE,    "packages-first-scan"},
    {HOTFIXES_TABLE,    "hotfixes-first-scan"},
    {PORTS_TABLE,       "ports-first-scan"},
    {PROCESSES_TABLE,   "processes-first-scan"},
    {OS_TABLE,          "system-first-scan"},
    {HW_TABLE,          "hardware-first-scan"}
};

static std::string GetItemId(const nlohmann::json& item, const std::vector<std::string>& idFields)
{
    Utils::HashData hash;

    for (const auto& field : idFields)
    {
        const auto& value{item.at(field)};

        if(!value.is_null())
        {
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

nlohmann::json Inventory::EcsData(const nlohmann::json& data, const std::string& table, bool createFields)
{
    nlohmann::json ret;
    if(table == HW_TABLE)
    {
        ret = EcsHardwareData(data, createFields);
    }
    else if (table == OS_TABLE)
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
    else if(table == HOTFIXES_TABLE)
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
        ret = data["observer"]["ingress"]["interface"]["name"].get<std::string>() + ":" + data["observer"]["ingress"]["interface"]["alias"].get<std::string>() + ":" + data["interface"]["type"].get<std::string>() + ":" + data["network"]["type"].get<std::string>() + ":" + data["host"]["ip"][0].get<std::string>();
    }
    return ret;
}

std::string Inventory::CalculateHashId(const nlohmann::json& data, const std::string& table)
{
    std::string primaryKey = GetPrimaryKeys(data, table);
    std::string baseId = AgentUUID() + ":" + primaryKey;

    Utils::HashData hash(Utils::HashType::Sha1);
    hash.update(baseId.c_str(), baseId.size());

    return Utils::asciiToHex(hash.hash());
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

                if(result == MODIFIED)
                {
                    msg["data"] = EcsData(item["new"], table);
                    msg["old_data"] = EcsData(item["old"], table, false);
                }
                else
                {
                    msg["data"] = EcsData(item, table);
                }

                msg["id"] = CalculateHashId(msg["data"], table);

                if (msg["id"].is_string() && msg["id"].get<std::string>().size() <= MAX_ID_SIZE)
                {
                    msg["data"]["@timestamp"] = m_scanTime;
                    const auto msgToSend{msg.dump()};
                    m_reportDiffFunction(msgToSend);
                }
                else
                {
                    LogWarn("Event discarded for exceeding maximum size allowed in id field.");
                    LogTrace("Event discarded: {}", msg.dump());
                }
            }
        }
        else
        {
            // LCOV_EXCL_START
            nlohmann::json msg;
            msg["type"] = table;
            msg["operation"] = OPERATION_MAP.at(result);

            if(result == MODIFIED)
            {
                msg["data"] = EcsData(data["new"], table);
                msg["old_data"] = EcsData(data["old"], table, false);
            }
            else
            {
                msg["data"] = EcsData(data, table);
            }

            msg["id"] = CalculateHashId(msg["data"], table);

            if (msg["id"].is_string() && msg["id"].get<std::string>().size() <= MAX_ID_SIZE)
            {
                msg["data"]["@timestamp"] = m_scanTime;
                const auto msgToSend{msg.dump()};
                m_reportDiffFunction(msgToSend);
            }
            else
            {
                LogWarn("Event discarded for exceeding maximum size allowed in id field.");
                LogTrace("Event discarded: {}", msg.dump());
            }

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

    std::unique_lock<std::mutex> lock{m_mutex};
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
    input["options"]["return_old_data"] = true;
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
        LogError("{}",std::string{ex.what()});
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
    , m_hardwareFirstScan { true }
    , m_systemFirstScan { true }
    , m_networksFirstScan { true }
    , m_packagesFirstScan { true }
    , m_portsFirstScan { true }
    , m_processesFirstScan { true }
    , m_hotfixesFirstScan { true }
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
        std::unique_lock<std::mutex> lock{m_mutex};
        m_stopping = false;
        m_spDBSync = std::make_unique<DBSync>(HostType::AGENT,
                                                DbEngineType::SQLITE3,
                                                dbPath,
                                                GetCreateStatement(),
                                                DbManagement::PERSISTENT);
        m_spNormalizer = std::make_unique<InvNormalizer>(normalizerConfigPath, normalizerType);
    }

   m_hardwareFirstScan  = ReadMetadata(TABLE_TO_KEY_MAP.at(HW_TABLE)).empty() ? false:true;
   m_systemFirstScan    = ReadMetadata(TABLE_TO_KEY_MAP.at(OS_TABLE)).empty() ? false:true;
   m_networksFirstScan  = ReadMetadata(TABLE_TO_KEY_MAP.at(NETWORKS_TABLE)).empty() ? false:true;
   m_packagesFirstScan  = ReadMetadata(TABLE_TO_KEY_MAP.at(PACKAGES_TABLE)).empty() ? false:true;
   m_portsFirstScan     = ReadMetadata(TABLE_TO_KEY_MAP.at(PORTS_TABLE)).empty() ? false:true;
   m_processesFirstScan = ReadMetadata(TABLE_TO_KEY_MAP.at(PROCESSES_TABLE)).empty() ? false:true;
   m_hotfixesFirstScan  = ReadMetadata(TABLE_TO_KEY_MAP.at(HOTFIXES_TABLE)).empty() ? false:true;

   if(m_hardwareFirstScan && !m_hardware)
   {
       DeleteMetadata(TABLE_TO_KEY_MAP.at(HW_TABLE));
       m_hardwareFirstScan = false;
   }

   if(m_systemFirstScan && !m_system)
   {
       DeleteMetadata(TABLE_TO_KEY_MAP.at(OS_TABLE));
       m_systemFirstScan = false;
   }

   if(m_networksFirstScan && !m_networks)
   {
       DeleteMetadata(TABLE_TO_KEY_MAP.at(NETWORKS_TABLE));
       m_networksFirstScan = false;
   }

   if(m_packagesFirstScan && !m_packages)
   {
       DeleteMetadata(TABLE_TO_KEY_MAP.at(PACKAGES_TABLE));
       m_packagesFirstScan = false;
   }

   if(m_portsFirstScan && !m_ports)
   {
       DeleteMetadata(TABLE_TO_KEY_MAP.at(PORTS_TABLE));
       m_portsFirstScan = false;
   }

   if(m_processesFirstScan && !m_processes)
   {
       DeleteMetadata(TABLE_TO_KEY_MAP.at(PROCESSES_TABLE));
       m_processesFirstScan = false;
   }

   if(m_hotfixesFirstScan && !m_hotfixes)
   {
       DeleteMetadata(TABLE_TO_KEY_MAP.at(HOTFIXES_TABLE));
       m_hotfixesFirstScan = false;
   }

    SyncLoop();
}

void Inventory::Destroy()
{
    std::unique_lock<std::mutex> lock{m_mutex};
    m_stopping = true;
    m_cv.notify_all();
}

nlohmann::json Inventory::EcsHardwareData(const nlohmann::json& originalData, bool createFields)
{
    nlohmann::json ret;

    auto setField = [&](const std::string& keyPath, const std::string& jsonKey, const std::optional<std::string>& defaultValue) {
        if (createFields || originalData.contains(jsonKey)) {
            nlohmann::json::json_pointer pointer(keyPath);
            if (originalData.contains(jsonKey) && originalData[jsonKey] != EMPTY_VALUE) {
                ret[pointer] = originalData[jsonKey];
            } else if (defaultValue.has_value()) {
                ret[pointer] = *defaultValue;
            } else {
                ret[pointer] = nullptr;
            }
        }
    };

    setField("/observer/serial_number", "board_serial", EMPTY_VALUE);
    setField("/host/cpu/name", "cpu_name", std::nullopt);
    setField("/host/cpu/cores", "cpu_cores", std::nullopt);
    setField("/host/cpu/speed", "cpu_mhz", std::nullopt);
    setField("/host/memory/total", "ram_total", std::nullopt);
    setField("/host/memory/free", "ram_free", std::nullopt);
    setField("/host/memory/used/percentage", "ram_usage", std::nullopt);

    return ret;
}

nlohmann::json Inventory::EcsSystemData(const nlohmann::json& originalData, bool createFields)
{
    nlohmann::json ret;

    auto setField = [&](const std::string& keyPath, const std::string& jsonKey, const std::optional<std::string>& defaultValue) {
        if (createFields || originalData.contains(jsonKey)) {
            nlohmann::json::json_pointer pointer(keyPath);
            if (originalData.contains(jsonKey) && originalData[jsonKey] != EMPTY_VALUE) {
                ret[pointer] = originalData[jsonKey];
            } else if (defaultValue.has_value()) {
                ret[pointer] = *defaultValue;
            } else {
                ret[pointer] = nullptr;
            }
        }
    };

    setField("/host/architecture", "architecture", std::nullopt);
    setField("/host/hostname", "hostname", std::nullopt);
    setField("/host/os/kernel", "os_build", std::nullopt);
    setField("/host/os/full", "os_codename", std::nullopt);
    setField("/host/os/name", "os_name", EMPTY_VALUE);
    setField("/host/os/platform", "os_platform", std::nullopt);
    setField("/host/os/version", "os_version", std::nullopt);
    setField("/host/os/type", "sysname", std::nullopt);

    return ret;
}

nlohmann::json Inventory::EcsPackageData(const nlohmann::json& originalData, bool createFields)
{
    nlohmann::json ret;

    auto setField = [&](const std::string& keyPath, const std::string& jsonKey, const std::optional<std::string>& defaultValue) {
        if (createFields || originalData.contains(jsonKey)) {
            nlohmann::json::json_pointer pointer(keyPath);
            if (originalData.contains(jsonKey) && originalData[jsonKey] != EMPTY_VALUE) {
                ret[pointer] = originalData[jsonKey];
            } else if (defaultValue.has_value()) {
                ret[pointer] = *defaultValue;
            } else {
                ret[pointer] = nullptr;
            }
        }
    };

    setField("/package/architecture", "architecture", EMPTY_VALUE);
    setField("/package/description", "description", std::nullopt);
    setField("/package/installed", "install_time", std::nullopt);
    setField("/package/name", "name", EMPTY_VALUE);
    setField("/package/path", "location", EMPTY_VALUE);
    setField("/package/size", "size", std::nullopt);
    setField("/package/type", "format", EMPTY_VALUE);
    setField("/package/version", "version", EMPTY_VALUE);

    return ret;
}

nlohmann::json Inventory::EcsProcessesData(const nlohmann::json& originalData, bool createFields)
{
    nlohmann::json ret;

    auto setField = [&](const std::string& keyPath, const std::string& jsonKey, const std::optional<std::string>& defaultValue) {
        if (createFields || originalData.contains(jsonKey)) {
            nlohmann::json::json_pointer pointer(keyPath);
            if (originalData.contains(jsonKey) && originalData[jsonKey] != EMPTY_VALUE) {
                ret[pointer] = originalData[jsonKey];
            } else if (defaultValue.has_value()) {
                ret[pointer] = *defaultValue;
            } else {
                ret[pointer] = nullptr;
            }
        }
    };

    setField("/process/pid", "pid", EMPTY_VALUE);
    setField("/process/name", "name", std::nullopt);
    setField("/process/parent/pid", "ppid", std::nullopt);
    setField("/process/command_line", "cmd", std::nullopt);
    setField("/process/args", "argvs", std::nullopt);
    setField("/process/user/id", "euser", std::nullopt);
    setField("/process/real_user/id", "ruser", std::nullopt);
    setField("/process/saved_user/id", "suser", std::nullopt);
    setField("/process/group/id", "egroup", std::nullopt);
    setField("/process/real_group/id", "rgroup", std::nullopt);
    setField("/process/saved_group/id", "sgroup", std::nullopt);
    setField("/process/start", "start_time", std::nullopt);
    setField("/process/thread/id", "tgid", std::nullopt);
    setField("/process/tty/char_device/major", "tty", std::nullopt);

    return ret;
}

nlohmann::json Inventory::EcsHotfixesData(const nlohmann::json& originalData, bool createFields){

    nlohmann::json ret;

    if(createFields || originalData.contains("hotfix"))
    {
        if(originalData.contains("hotfix") && originalData["hotfix"] != EMPTY_VALUE)
        {
            ret["package"]["hotfix"]["name"] = originalData["hotfix"];
        }
        else
        {
            ret["package"]["hotfix"]["name"] = EMPTY_VALUE;
        }
    }

    return ret;
}

nlohmann::json Inventory::EcsPortData(const nlohmann::json& originalData, bool createFields)
{
    nlohmann::json ret;

    auto setField = [&](const std::string& keyPath, const std::string& jsonKey, const std::optional<std::string>& defaultValue) {
        if (createFields || originalData.contains(jsonKey)) {
            nlohmann::json::json_pointer pointer(keyPath);
            if (originalData.contains(jsonKey) && originalData[jsonKey] != EMPTY_VALUE) {
                ret[pointer] = originalData[jsonKey];
            } else if (defaultValue.has_value()) {
                ret[pointer] = *defaultValue;
            } else {
                ret[pointer] = nullptr;
            }
        }
    };

    auto setFieldArray = [&](const std::string& destPath, const std::string& sourceKey) {
        if (createFields || originalData.contains(sourceKey)) {
            nlohmann::json::json_pointer destPointer(destPath);
            ret[destPointer.parent_pointer()][destPointer.back()] = nlohmann::json::array();
            nlohmann::json& destArray = ret[destPointer];

            if (originalData.contains(sourceKey) &&
                !originalData[sourceKey].empty() &&
                !originalData[sourceKey].is_null() &&
                originalData[sourceKey] != EMPTY_VALUE)
            {
                destArray.push_back(originalData[sourceKey]);
            }
        }
    };

    setField("/network/protocol", "protocol",  EMPTY_VALUE);
    setFieldArray("/source/ip", "local_ip");
    setField("/source/port", "local_port",  EMPTY_VALUE);
    setFieldArray("/destination/ip", "remote_ip");
    setField("/destination/port", "remote_port",  std::nullopt);
    setField("/host/network/egress/queue", "tx_queue",  std::nullopt);
    setField("/host/network/ingress/queue", "rx_queue",  std::nullopt);
    setField("/file/inode", "inode",  EMPTY_VALUE);
    setField("/interface/state", "state",  std::nullopt);
    setField("/process/pid", "pid",  std::nullopt);
    setField("/process/name", "process",  std::nullopt);

    return ret;
}

nlohmann::json Inventory::EcsNetworkData(const nlohmann::json& originalData, bool createFields)
{
    nlohmann::json ret;

    auto setField = [&](const std::string& keyPath, const std::string& jsonKey, const std::optional<std::string>& defaultValue) {
        if (createFields || originalData.contains(jsonKey)) {
            nlohmann::json::json_pointer pointer(keyPath);
            if (originalData.contains(jsonKey) && originalData[jsonKey] != EMPTY_VALUE) {
                ret[pointer] = originalData[jsonKey];
            } else if (defaultValue.has_value()) {
                ret[pointer] = *defaultValue;
            } else {
                ret[pointer] = nullptr;
            }
        }
    };

    auto setFieldArray = [&](const std::string& destPath, const std::string& sourceKey) {
        if (createFields || originalData.contains(sourceKey)) {
            nlohmann::json::json_pointer destPointer(destPath);
            ret[destPointer.parent_pointer()][destPointer.back()] = nlohmann::json::array();
            nlohmann::json& destArray = ret[destPointer];

            if (originalData.contains(sourceKey) &&
                !originalData[sourceKey].empty() &&
                !originalData[sourceKey].is_null() &&
                originalData[sourceKey] != EMPTY_VALUE)
            {
                destArray.push_back(originalData[sourceKey]);
            }
        }
    };

    setFieldArray("/host/ip", "address");
    setField("/host/mac", "mac", std::nullopt);
    setField("/host/network/egress/bytes", "tx_bytes", std::nullopt);
    setField("/host/network/egress/packets", "tx_packets", std::nullopt);
    setField("/host/network/ingress/bytes", "rx_bytes", std::nullopt);
    setField("/host/network/ingress/packets", "rx_packets", std::nullopt);
    setField("/host/network/egress/drops", "tx_dropped", std::nullopt);
    setField("/host/network/egress/errors", "tx_errors", std::nullopt);
    setField("/host/network/ingress/drops", "rx_dropped", std::nullopt);
    setField("/host/network/ingress/errors", "rx_errors", std::nullopt);
    setField("/interface/mtu", "mtu", std::nullopt);
    setField("/interface/state", "state", std::nullopt);
    setField("/interface/type", "iface_type", EMPTY_VALUE);
    setFieldArray("/network/netmask", "netmask");
    setFieldArray("/network/gateway", "gateway");
    setFieldArray("/network/broadcast", "broadcast");
    setField("/network/dhcp", "dhcp", std::nullopt);
    setField("/network/type", "proto_type", EMPTY_VALUE);
    setField("/network/metric", "metric", std::nullopt);
    /* TODO this field should include http or https, it's related to an application not to a interface */
    if (createFields)
    {
        ret["network"]["protocol"] = nullptr;
    }
    setField("/observer/ingress/interface/alias", "adapter", EMPTY_VALUE);
    setField("/observer/ingress/interface/name", "iface", EMPTY_VALUE);

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

        if(!m_hardwareFirstScan){
            WriteMetadata(TABLE_TO_KEY_MAP.at(HW_TABLE), Utils::getCurrentISO8601());
            m_hardwareFirstScan = true;
        }
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

        if(!m_systemFirstScan){
            WriteMetadata(TABLE_TO_KEY_MAP.at(OS_TABLE), Utils::getCurrentISO8601());
            m_systemFirstScan = true;
        }
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

        if(!m_networksFirstScan){
            WriteMetadata(TABLE_TO_KEY_MAP.at(NETWORKS_TABLE), Utils::getCurrentISO8601());
            m_networksFirstScan = true;
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

        std::unique_lock<std::mutex> lock{m_mutex};
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

            input["table"] = PACKAGES_TABLE;
            m_spNormalizer->Normalize("packages", rawData);
            m_spNormalizer->RemoveExcluded("packages", rawData);

            if (!rawData.empty())
            {
                input["data"] = nlohmann::json::array( { rawData } );
                input["options"]["return_old_data"] = true;
                txn.syncTxnRow(input);
            }
        });
        txn.getDeletedRows(callback);

        if(!m_packagesFirstScan){
            WriteMetadata(TABLE_TO_KEY_MAP.at(PACKAGES_TABLE), Utils::getCurrentISO8601());
            m_packagesFirstScan = true;
        }

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

        if(!m_hotfixesFirstScan){
           WriteMetadata(TABLE_TO_KEY_MAP.at(HOTFIXES_TABLE), Utils::getCurrentISO8601());
           m_hotfixesFirstScan = true;
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

        if(!m_portsFirstScan){
           WriteMetadata(TABLE_TO_KEY_MAP.at(PORTS_TABLE), Utils::getCurrentISO8601());
           m_portsFirstScan = true;
        }
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
        std::unique_lock<std::mutex> lock{m_mutex};
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
            input["options"]["return_old_data"] = true;

            txn.syncTxnRow(input);
        });
        txn.getDeletedRows(callback);

        if(!m_processesFirstScan){
           WriteMetadata(TABLE_TO_KEY_MAP.at(PROCESSES_TABLE), Utils::getCurrentISO8601());
           m_processesFirstScan = true;
        }

        LogTrace( "Ending processes scan");
    }
}

void Inventory::Scan()
{
    LogInfo("Starting evaluation.");
    m_scanTime = Utils::getCurrentISO8601();

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
            std::unique_lock<std::mutex> lock{m_mutex};
            m_cv.wait_for(lock,
                std::chrono::milliseconds{m_intervalValue}, [&]() { return m_stopping; } );
        }
        Scan();
    }
    std::unique_lock<std::mutex> lock{m_mutex};
    m_spDBSync.reset(nullptr);
}

void Inventory::WriteMetadata(const std::string &key, const std::string &value){
    auto insertQuery
    {
        InsertQuery::builder()
        .table(MD_TABLE)
        .data({{"key", key}, {"value", value}})
        .build()
    };
    m_spDBSync->insertData(insertQuery.query());
}

std::string Inventory::ReadMetadata(const std::string &key) {
    std::string result;
    std::string filter = "WHERE key = '" + key + "'";
    auto selectQuery = SelectQuery::builder()
        .table("metadata")
        .columnList({"key", "value"})
        .rowFilter(filter)
        .build();

    auto callback = [&result](ReturnTypeCallback returnTypeCallback, const nlohmann::json& resultData) {
        (void)returnTypeCallback;
        if (resultData.is_object() && resultData.contains("key") && resultData.contains("value")) {
            result = resultData["value"];
        }
    };

    m_spDBSync->selectRows(selectQuery.query(), callback);

    return result;
}

void Inventory::DeleteMetadata(const std::string &key){
    auto deleteQuery
    {
        DeleteQuery::builder()
        .table("metadata")
        .data({{"key", key}})
        .rowFilter("")
        .build()
    };
    m_spDBSync->deleteRows(deleteQuery.query());
}

void Inventory::CleanMetadata()
{
    DBSync::initialize(LogErrorInventory);

    try
    {
        {
            std::unique_lock<std::mutex> lock{m_mutex};
            m_spDBSync = std::make_unique<DBSync>(HostType::AGENT,
                                                    DbEngineType::SQLITE3,
                                                    m_dbFilePath,
                                                    METADATA_SQL_STATEMENT,
                                                    DbManagement::PERSISTENT);
            for (const auto& key : TABLE_TO_KEY_MAP) {
                if(!ReadMetadata(key.second).empty()){
                    DeleteMetadata(key.second);
                }
            }
            m_spDBSync.reset();
        }
    }
    catch (const std::exception& ex)
    {
        LogErrorInventory(ex.what());
    }
}
