#pragma once

#include <chrono>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <string>
#include <memory>
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
        void Setup(const configuration::ConfigurationParser& configurationParser);
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

    private:
        Inventory();
        ~Inventory() = default;
        Inventory(const Inventory&) = delete;
        Inventory& operator=(const Inventory&) = delete;

        void Destroy();

        std::string GetCreateStatement() const;
        nlohmann::json EcsProcessesData(const nlohmann::json& originalData);
        nlohmann::json EcsSystemData(const nlohmann::json& originalData);
        nlohmann::json GetOSData();
        nlohmann::json EcsHardwareData(const nlohmann::json& originalData);
        nlohmann::json EcsPackageData(const nlohmann::json& originalData);
        nlohmann::json GetHardwareData();
        nlohmann::json GetNetworkData();
        nlohmann::json GetPortsData();

        void UpdateChanges(const std::string& table, const nlohmann::json& values);
        void NotifyChange(ReturnTypeCallback result, const nlohmann::json& data, const std::string& table);
        void TryCatchTask(const std::function<void()>& task) const;
        void ScanHardware();
        void ScanOs();
        void ScanNetwork();
        void ScanPackages();
        void ScanHotfixes();
        void ScanPorts();
        void ScanProcesses();
        void Scan();
        void SyncLoop(std::unique_lock<std::mutex>& lock);
        void ShowConfig();
        cJSON * Dump();
        static void LogErrorInventory(const std::string& log);
        nlohmann::json EcsData(const nlohmann::json& data, const std::string& table);
        std::string GetPrimaryKeys(const nlohmann::json& data, const std::string& table);
        std::string CalculateBase64Id(const nlohmann::json& data, const std::string& table);

        const std::string                           m_moduleName {"inventory"};
        std::shared_ptr<ISysInfo>                   m_spInfo;
        std::function<void(const std::string&)>     m_reportDiffFunction;
        bool                                        m_enabled;          // Main switch
        std::chrono::seconds                        m_intervalValue;    // Scan interval
        bool                                        m_scanOnStart;      // Scan always on start
        bool                                        m_hardware;         // Hardware inventory
        bool                                        m_os;               // OS inventory
        bool                                        m_network;          // Network inventory
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
};
