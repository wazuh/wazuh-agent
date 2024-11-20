#include <inventory.hpp>

#include <cjson/cJSON.h>
#include <config.h>
#include <defs.h>
#include <logger.hpp>
#include <sysInfo.hpp>


void Inventory::Start() {

    if (!m_enabled) {
        LogInfo("Module disabled. Exiting...");
        return;
    }

    LogInfo("Starting inventory.");

    ShowConfig();

    DBSync::initialize(LogErrorInventory);

    try
    {
        Inventory::Instance().Init(std::make_shared<SysInfo>(),
                                    [this](const std::string& diff) { this->SendDeltaEvent(diff); },
                                    INVENTORY_DB_DISK_PATH,
                                    INVENTORY_NORM_CONFIG_DISK_PATH,
                                    INVENTORY_NORM_TYPE);
    }
    catch (const std::exception& ex)
    {
        LogErrorInventory(ex.what());
    }

    LogInfo("Module finished.");

}

void Inventory::Setup(const configuration::ConfigurationParser& configurationParser) {

    m_enabled = configurationParser.GetConfig<bool>( "inventory", "enabled").value_or(config::inventory::DEFAULT_ENABLED);
    m_intervalValue = configurationParser.GetConfig<std::time_t>("inventory", "interval").value_or(config::inventory::DEFAULT_INTERVAL);
    m_scanOnStart = configurationParser.GetConfig<bool>("inventory", "scan_on_start").value_or(config::inventory::DEFAULT_SCAN_ON_START);
    m_hardware = configurationParser.GetConfig<bool>("inventory", "hardware").value_or(config::inventory::DEFAULT_HARDWARE);
    m_os = configurationParser.GetConfig<bool>("inventory", "os").value_or(config::inventory::DEFAULT_OS);
    m_network = configurationParser.GetConfig<bool>("inventory", "network").value_or(config::inventory::DEFAULT_NETWORK);
    m_packages = configurationParser.GetConfig<bool>("inventory", "packages").value_or(config::inventory::DEFAULT_PACKAGES);
    m_ports = configurationParser.GetConfig<bool>("inventory", "ports").value_or(config::inventory::DEFAULT_PORTS);
    m_portsAll = configurationParser.GetConfig<bool>("inventory", "ports_all").value_or(config::inventory::DEFAULT_PORTS_ALL);
    m_processes = configurationParser.GetConfig<bool>("inventory", "processes").value_or(config::inventory::DEFAULT_PROCESSES);
    m_hotfixes = configurationParser.GetConfig<bool>("inventory", "hotfixes").value_or(config::inventory::DEFAULT_HOTFIXES);
}

void Inventory::Stop() {
    LogInfo("Module stopped.");
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

void Inventory::SendDeltaEvent(const std::string& data) {

    const auto jsonData = nlohmann::json::parse(data);
    auto metadata = nlohmann::json::object();

    metadata["module"] = Name();
    metadata["type"] = jsonData["type"];
    metadata["operation"] = jsonData["operation"];
    metadata["id"] = jsonData["id"];

    const Message statefulMessage{ MessageType::STATEFUL, jsonData["data"], Name(), jsonData["type"], metadata.dump() };

    if(!m_pushMessage(statefulMessage)) {
        LogWarn("Stateful event can't be pushed into the message queue: {}", data);
    }
    else {
        LogTrace("Stateful event queued: {}, metadata {}", data, metadata.dump());
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
    if (m_network) cJSON_AddStringToObject(invJson,"network","yes"); else cJSON_AddStringToObject(invJson,"network","no");
    if (m_os) cJSON_AddStringToObject(invJson,"os","yes"); else cJSON_AddStringToObject(invJson,"os","no");
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

void Inventory::LogErrorInventory(const std::string& log)
{
    LogError("{}", log.c_str());
}
