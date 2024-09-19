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
#include <logging_helper.h>
#include <commonDefs.h>
#include <dbsync.hpp>
#include <inventoryNormalizer.hpp>
#include <multitype_queue.hpp>

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
        std::string Command(const std::string & query);
        const std::string& Name() const { return m_moduleName; };
        void SetMessageQueue(const std::shared_ptr<IMultiTypeQueue> queue);

        void Init(const std::shared_ptr<ISysInfo>& spInfo,
                    const std::function<void(const std::string&)> reportDiffFunction,
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
        nlohmann::json GetOSData();
        nlohmann::json GetHardwareData();
        nlohmann::json GetNetworkData();
        nlohmann::json GetPortsData();

        void UpdateChanges(const std::string& table, const nlohmann::json& values);
        void NotifyChange(ReturnTypeCallback result, const nlohmann::json& data, const std::string& table);
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
        static void Log(const modules_log_level_t level, const std::string& log);
        static void LogError(const std::string& log);

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
        std::shared_ptr<IMultiTypeQueue>            m_messageQueue;

};
