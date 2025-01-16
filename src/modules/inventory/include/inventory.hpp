#pragma once

#include <chrono>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <string>
#include <ctime>
#include <stack>

#include <sysInfoInterface.hpp>
#include <configuration_parser.hpp>
#include <commonDefs.h>
#include <dbsync.hpp>
#include <inventoryNormalizer.hpp>
#include <multitype_queue.hpp>

#include <moduleWrapper.hpp>

#include <boost/asio/awaitable.hpp>

class Inventory {
    public:
        static Inventory& Instance()
        {
            static Inventory s_instance;
            return s_instance;
        }
        void Start();
        void Setup(std::shared_ptr<const configuration::ConfigurationParser> configurationParser);
        void Stop();
        Co_CommandExecutionResult ExecuteCommand(const std::string command, const nlohmann::json parameters);
        const std::string& Name() const { return m_moduleName; };
        void SetPushMessageFunction(const std::function<int(Message)>& pushMessage);

        void Init(const std::shared_ptr<ISysInfo>& spInfo,
                    const std::function<void(const std::string&)>& reportDiffFunction,
                    const std::string& dbPath,
                    const std::string& normalizerConfigPath,
                    const std::string& normalizerType);
        virtual void SendDeltaEvent(const std::string& data);

        const std::string& AgentUUID() const { return m_agentUUID; };
        void SetAgentUUID(const std::string& agentUUID) {
            m_agentUUID = agentUUID;
        }

    private:
        Inventory();
        ~Inventory() = default;
        Inventory(const Inventory&) = delete;
        Inventory& operator=(const Inventory&) = delete;

        void Destroy();

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
        void NotifyChange(ReturnTypeCallback result, const nlohmann::json& data, const std::string& table);
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
        cJSON * Dump() const;
        static void LogErrorInventory(const std::string& log);
        nlohmann::json EcsData(const nlohmann::json& data, const std::string& table, bool createFields = true);
        std::string GetPrimaryKeys(const nlohmann::json& data, const std::string& table);
        std::string CalculateHashId(const nlohmann::json& data, const std::string& table);
        nlohmann::json AddPreviousFields(nlohmann::json& current, const nlohmann::json& previous);
        nlohmann::json GenerateStatelessEvent(const std::string& operation, const std::string& type, const nlohmann::json& data);

        void WriteMetadata(const std::string &key, const std::string &value);
        std::string ReadMetadata(const std::string &key);
        void DeleteMetadata(const std::string &key);
        void CleanMetadata();

        const std::string                           m_moduleName {"inventory"};
        std::string                                 m_agentUUID {""};   // Agent UUID
        std::shared_ptr<ISysInfo>                   m_spInfo;
        std::function<void(const std::string&)>     m_reportDiffFunction;
        bool                                        m_enabled;          // Main switch
        std::string                                 m_dbFilePath;       // Database path
        std::time_t                                 m_intervalValue;    // Scan interval
        bool                                        m_scanOnStart;      // Scan always on start
        bool                                        m_hardware;         // Hardware inventory
        bool                                        m_system;           // System inventory
        bool                                        m_networks;         // Networks inventory
        bool                                        m_packages;         // Installed packages inventory
        bool                                        m_ports;            // Opened ports inventory
        bool                                        m_portsAll;         // Scan only listening ports or all
        bool                                        m_processes;        // Running processes inventory
        bool                                        m_hotfixes;         // Windows hotfixes installed
        bool                                        m_stopping;
        bool                                        m_notify;
        std::unique_ptr<DBSync>                     m_spDBSync;
        std::condition_variable                     m_cv;
        std::mutex                                  m_mutex;
        std::unique_ptr<InvNormalizer>              m_spNormalizer;
        std::string                                 m_scanTime;
        std::function<int(Message)>                 m_pushMessage;
        bool                                        m_hardwareFirstScan;  // Hardware first scan flag
        bool                                        m_systemFirstScan;    // System first scan flag
        bool                                        m_networksFirstScan;  // Networks first scan flag
        bool                                        m_packagesFirstScan;  // Installed packages first scan flag
        bool                                        m_portsFirstScan;     // Opened ports first scan flag
        bool                                        m_processesFirstScan; // Running processes first scan flag
        bool                                        m_hotfixesFirstScan;  // Windows hotfixes installed first scan flag
};
