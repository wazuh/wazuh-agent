#include <inventory.hpp>

#include <cjson/cJSON.h>
#include <config.h>
#include <defs.h>
#include <logger.hpp>
#include <sysInfo.hpp>
#include <timeHelper.h>

void Inventory::Start() {

    if (!m_enabled) {
        LogInfo("Inventory module is disabled.");
        return;
    }

    LogInfo("Inventory module started.");

    ShowConfig();

    DBSync::initialize(LogErrorInventory);

    try
    {
        Inventory::Instance().Init(std::make_shared<SysInfo>(),
                                    [this](const std::string& diff) { this->SendDeltaEvent(diff, false); },
                                    m_dbFilePath,
                                    INVENTORY_NORM_CONFIG_DISK_PATH,
                                    INVENTORY_NORM_TYPE);
    }
    catch (const std::exception& ex)
    {
        LogErrorInventory(ex.what());
    }

    LogInfo("Inventory module stopped.");
}

void Inventory::Setup(std::shared_ptr<const configuration::ConfigurationParser> configurationParser) {
    if (!configurationParser) {
        LogError("Invalid Configuration Parser passed to setup, module set to disabled.");
        m_enabled = false;
        return;
    }

    m_enabled = configurationParser->GetConfig<bool>( "inventory", "enabled").value_or(config::inventory::DEFAULT_ENABLED);
    m_dbFilePath = configurationParser->GetConfig<std::string>("agent", "path.data").value_or(config::DEFAULT_DATA_PATH) + "/" + INVENTORY_DB_DISK_NAME;
    m_intervalValue = configurationParser->GetConfig<std::time_t>("inventory", "interval").value_or(config::inventory::DEFAULT_INTERVAL);
    m_scanOnStart = configurationParser->GetConfig<bool>("inventory", "scan_on_start").value_or(config::inventory::DEFAULT_SCAN_ON_START);
    m_hardware = configurationParser->GetConfig<bool>("inventory", "hardware").value_or(config::inventory::DEFAULT_HARDWARE);
    m_system = configurationParser->GetConfig<bool>("inventory", "system").value_or(config::inventory::DEFAULT_OS);
    m_networks = configurationParser->GetConfig<bool>("inventory", "networks").value_or(config::inventory::DEFAULT_NETWORK);
    m_packages = configurationParser->GetConfig<bool>("inventory", "packages").value_or(config::inventory::DEFAULT_PACKAGES);
    m_ports = configurationParser->GetConfig<bool>("inventory", "ports").value_or(config::inventory::DEFAULT_PORTS);
    m_portsAll = configurationParser->GetConfig<bool>("inventory", "ports_all").value_or(config::inventory::DEFAULT_PORTS_ALL);
    m_processes = configurationParser->GetConfig<bool>("inventory", "processes").value_or(config::inventory::DEFAULT_PROCESSES);
    m_hotfixes = configurationParser->GetConfig<bool>("inventory", "hotfixes").value_or(config::inventory::DEFAULT_HOTFIXES);
}

void Inventory::Stop() {
    LogInfo("Inventory module stopping...");
    Inventory::Instance().Destroy();
}

// NOLINTNEXTLINE(performance-unnecessary-value-param)
Co_CommandExecutionResult Inventory::ExecuteCommand(const std::string command, [[maybe_unused]] const nlohmann::json parameters) {
    LogInfo("Command: {}", command);
    co_return module_command::CommandExecutionResult{module_command::Status::SUCCESS, "OK"};
}

void Inventory::SetPushMessageFunction(const std::function<int(Message)>& pushMessage) {
    m_pushMessage = pushMessage;
}

void Inventory::SendDeltaEvent(const std::string& data, bool isFirstScan) {

    const auto jsonData = nlohmann::json::parse(data);
    auto metadata = nlohmann::json::object();

    metadata["module"] = Name();
    metadata["type"] = jsonData["type"];
    metadata["operation"] = jsonData["operation"];
    metadata["id"] = jsonData["id"];

    const Message statefulMessage{ MessageType::STATEFUL, metadata["operation"] == "delete" ? "{}"_json : jsonData["data"], Name(), jsonData["type"], metadata.dump() };

    if(!m_pushMessage(statefulMessage)) {
        LogWarn("Stateful event can't be pushed into the message queue: {}", data);
    }
    else {
        LogTrace("Stateful event queued: {}, metadata {}", data, metadata.dump());
    }

    if(!isFirstScan) {
        auto statelessMetadata = nlohmann::json::object();

        statelessMetadata["module"] = Name();
        statelessMetadata["type"] = jsonData["type"];
        statelessMetadata["operation"] = jsonData["operation"];
        statelessMetadata["id"] = jsonData["id"];

        auto statelessJsonData = nlohmann::json::object();

        statelessJsonData["log"]["file"]["path"] = "log->file->path";
        statelessJsonData["tags"] = nlohmann::json::array({"mvp"});
        statelessJsonData["event"]["original"] = jsonData["data"];
        statelessJsonData["event"]["module"] = m_moduleName;
        statelessJsonData["event"]["provider"] = "syslog";

        statelessJsonData["event"]["created"] = Utils::getCurrentISO8601();

        const Message statelessMessage{ MessageType::STATELESS, statelessJsonData, Name(), statelessMetadata["type"], statelessMetadata.dump() };

        if(!m_pushMessage(statelessMessage)) {
            LogWarn("Stateless event can't be pushed into the message queue: {}", statelessMetadata.dump());
        }
        else {
            LogTrace("Stateless event queued: {}, metadata {}", statelessMetadata.dump(), statelessMetadata.dump());
        }
    }
}

void Inventory::ShowConfig()
{
    cJSON * configJson = Dump();
    if (configJson) {
        char * configString = cJSON_PrintUnformatted(configJson);
        if (configString) {
            LogTrace("{}", configString);
            cJSON_free(configString);
        }
        cJSON_Delete(configJson);
    }
}

cJSON * Inventory::Dump() const
{
    cJSON *rootJson = cJSON_CreateObject();
    cJSON *invJson = cJSON_CreateObject();

    // System provider values
    if (m_enabled) cJSON_AddStringToObject(invJson,"enabled","yes"); else cJSON_AddStringToObject(invJson,"enabled","no");
    if (m_scanOnStart) cJSON_AddStringToObject(invJson,"scan-on-start","yes"); else cJSON_AddStringToObject(invJson,"scan-on-start","no");
    cJSON_AddNumberToObject(invJson, "interval", static_cast<double>(m_intervalValue));
    if (m_networks) cJSON_AddStringToObject(invJson,"networks","yes"); else cJSON_AddStringToObject(invJson,"networks","no");
    if (m_system) cJSON_AddStringToObject(invJson,"system","yes"); else cJSON_AddStringToObject(invJson,"system","no");
    if (m_hardware) cJSON_AddStringToObject(invJson,"hardware","yes"); else cJSON_AddStringToObject(invJson,"hardware","no");
    if (m_packages) cJSON_AddStringToObject(invJson,"packages","yes"); else cJSON_AddStringToObject(invJson,"packages","no");
    if (m_ports) cJSON_AddStringToObject(invJson,"ports","yes"); else cJSON_AddStringToObject(invJson,"ports","no");
    if (m_portsAll) cJSON_AddStringToObject(invJson,"ports_all","yes"); else cJSON_AddStringToObject(invJson,"ports_all","no");
    if (m_processes) cJSON_AddStringToObject(invJson,"processes","yes"); else cJSON_AddStringToObject(invJson,"processes","no");
#ifdef WIN32
    if (m_hotfixes) cJSON_AddStringToObject(invJson,"hotfixes","yes"); else cJSON_AddStringToObject(invJson,"hotfixes","no");
#endif

    cJSON_AddItemToObject(rootJson,"inventory",invJson);

    return rootJson;
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

void Inventory::LogErrorInventory(const std::string& log)
{
    LogError("{}", log.c_str());
}
