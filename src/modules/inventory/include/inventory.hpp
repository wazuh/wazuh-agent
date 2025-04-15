#pragma once

#include <chrono>
#include <condition_variable>
#include <ctime>
#include <memory>
#include <mutex>
#include <stack>
#include <string>
#include <thread>

#include <commonDefs.h>
#include <dbsync.hpp>
#include <inventoryNormalizer.hpp>
#include <sysInfoInterface.hpp>

#include <command_entry.hpp>
#include <imodule.hpp>
#include <message.hpp>

#include <boost/asio/awaitable.hpp>

class Inventory : public IModule
{
public:
    const std::string INVENTORY_DB_DISK_NAME = "inventory.db";
    const std::string INVENTORY_NORM_CONFIG_NAME = "norm_config.json";

    Inventory();
    ~Inventory() = default;
    Inventory(const Inventory&) = delete;
    Inventory& operator=(const Inventory&) = delete;

    /// @copydoc IModule::Run
    void Run() override;

    /// @copydoc IModule::Setup
    void Setup(std::shared_ptr<const configuration::ConfigurationParser> configurationParser) override;

    /// @copydoc IModule::Stop
    void Stop() override;

    /// @copydoc IModule::ExecuteCommand
    Co_CommandExecutionResult ExecuteCommand(const std::string command, const nlohmann::json parameters) override;

    /// @copydoc IModule::Name
    const std::string& Name() const override;

    /// @copydoc IModule::SetPushMessageFunction
    void SetPushMessageFunction(const std::function<int(Message)>& pushMessage) override;

    void Init(const std::shared_ptr<ISysInfo>& spInfo,
              const std::function<void(const std::string&)>& reportDiffFunction,
              const std::string& dbPath,
              const std::string& normalizerConfigPath,
              const std::string& normalizerType);

    virtual void PushMessage(const std::string& data);

    const std::string& AgentUUID() const;

    void SetAgentUUID(const std::string& agentUUID);

private:
    std::string GetCreateStatement() const;
    nlohmann::json EcsProcessesData(const nlohmann::json& originalData, bool createFields = true);
    nlohmann::json EcsSystemData(const nlohmann::json& originalData, bool createFields = true);
    nlohmann::json EcsHotfixesData(const nlohmann::json& originalData, bool createFields = true);
    nlohmann::json EcsHardwareData(const nlohmann::json& originalData, bool createFields = true);
    nlohmann::json EcsPackageData(const nlohmann::json& originalData, bool createFields = true);
    nlohmann::json EcsPortData(const nlohmann::json& originalData, bool createFields = true);
    nlohmann::json EcsNetworkData(const nlohmann::json& originalData, bool createFields = true);
    nlohmann::json GetNetworkData();
    nlohmann::json GetPortsData();

    void UpdateChanges(const std::string& table, const nlohmann::json& values, const bool isFirstScan);
    void NotifyChange(ReturnTypeCallback result,
                      const nlohmann::json& data,
                      const std::string& table,
                      const bool isFirstScan);
    void ProcessEvent(ReturnTypeCallback result,
                      const nlohmann::json& item,
                      const std::string& table,
                      const bool isFirstScan);
    nlohmann::json GenerateMessage(ReturnTypeCallback result, const nlohmann::json& item, const std::string& table);
    void NotifyEvent(ReturnTypeCallback result,
                     nlohmann::json& msg,
                     const nlohmann::json& item,
                     const std::string& table,
                     const bool isFirstScan);

    void TryCatchTask(const std::function<void()>& task) const;
    void ScanHardware();
    void ScanSystem();
    void ScanNetwork();
    void ScanPackages();
    void ScanHotfixes();
    void ScanPorts();
    void ScanProcesses();
    void Scan();
    void SyncLoop();
    void ShowConfig();
    cJSON* Dump() const;
    nlohmann::json EcsData(const nlohmann::json& data, const std::string& table, bool createFields = true);
    std::string GetPrimaryKeys(const nlohmann::json& data, const std::string& table);
    std::string CalculateHashId(const nlohmann::json& data, const std::string& table);
    nlohmann::json AddPreviousFields(nlohmann::json& current, const nlohmann::json& previous);
    nlohmann::json
    GenerateStatelessEvent(const std::string& operation, const std::string& type, const nlohmann::json& data);

    void WriteMetadata(const std::string& key, const std::string& value);
    std::string ReadMetadata(const std::string& key);
    void DeleteMetadata(const std::string& key);
    void CleanMetadata();

    void SetJsonField(nlohmann::json& target,
                      const nlohmann::json& source,
                      const std::string& keyPath,
                      const std::string& jsonKey,
                      const std::optional<std::string>& defaultValue,
                      bool createFields);
    void SetJsonFieldArray(nlohmann::json& target,
                           const nlohmann::json& source,
                           const std::string& destPath,
                           const std::string& sourceKey,
                           bool createFields);

    const std::string m_moduleName {"inventory"};
    std::string m_agentUUID {""}; // Agent UUID
    std::shared_ptr<ISysInfo> m_spInfo;
    std::function<void(const std::string&)> m_reportDiffFunction;
    bool m_enabled;               // Main switch
    std::string m_dbFilePath;     // Database path
    std::string m_normConfigPath; // Normalizer JSON path
    std::time_t m_intervalValue;  // Scan interval
    bool m_scanOnStart;           // Scan always on start
    bool m_hardware;              // Hardware inventory
    bool m_system;                // System inventory
    bool m_networks;              // Networks inventory
    bool m_packages;              // Installed packages inventory
    bool m_ports;                 // Opened ports inventory
    bool m_portsAll;              // Scan only listening ports or all
    bool m_processes;             // Running processes inventory
    bool m_hotfixes;              // Windows hotfixes installed
    std::atomic<bool> m_stopping;
    bool m_notify;
    std::unique_ptr<DBSync> m_spDBSync;
    std::condition_variable m_cv;
    std::mutex m_mutex;
    std::unique_ptr<InvNormalizer> m_spNormalizer;
    std::string m_scanTime;
    std::function<int(Message)> m_pushMessage;
    bool m_hardwareFirstScan;  // Hardware first scan flag
    bool m_systemFirstScan;    // System first scan flag
    bool m_networksFirstScan;  // Networks first scan flag
    bool m_packagesFirstScan;  // Installed packages first scan flag
    bool m_portsFirstScan;     // Opened ports first scan flag
    bool m_processesFirstScan; // Running processes first scan flag
    bool m_hotfixesFirstScan;  // Windows hotfixes installed first scan flag
};
