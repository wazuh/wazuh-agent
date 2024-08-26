#pragma once

#include <chrono>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <string>
#include <memory>
#include "sysInfoInterface.hpp"
#include "configuration.hpp"
#include "logging_helper.h"
#include "commonDefs.h"
#include "dbsync.hpp"
#include "inventoryNormalizer.hpp"

class Inventory {
    public:
        static Inventory& instance()
        {
            static Inventory s_instance;
            return s_instance;
        }

        void *run();
        int setup(const Configuration & config);
        void stop();
        std::string command(const std::string & query);
        std::string name() const;

    private:
        Inventory();
        ~Inventory() = default;
        Inventory(const Inventory&) = delete;
        Inventory& operator=(const Inventory&) = delete;

        void init(const std::shared_ptr<ISysInfo>& spInfo,
                    const std::string& dbPath,
                    const std::string& normalizerConfigPath,
                    const std::string& normalizerType);

        void destroy();

        std::string getCreateStatement() const;
        nlohmann::json getOSData();
        nlohmann::json getHardwareData();
        nlohmann::json getNetworkData();
        nlohmann::json getPortsData();

        void updateChanges(const std::string& table,
                            const nlohmann::json& values);
        void notifyChange(ReturnTypeCallback result,
                            const nlohmann::json& data,
                            const std::string& table);
        void scanHardware();
        void scanOs();
        void scanNetwork();
        void scanPackages();
        void scanHotfixes();
        void scanPorts();
        void scanProcesses();
        void syncOs();
        void syncHardware();
        void syncNetwork();
        void syncPackages();
        void syncHotfixes();
        void syncPorts();
        void syncProcesses();
        void scan();
        void syncLoop(std::unique_lock<std::mutex>& lock);
        void syncAlgorithm();

        static void log(const modules_log_level_t level, const std::string& log);
        static void logError(const std::string& log);

        std::shared_ptr<ISysInfo>                   m_spInfo;
        std::function<void(const std::string&)>     m_reportSyncFunction;
        std::chrono::seconds                        m_intervalValue;
        std::chrono::seconds                        m_currentIntervalValue;
        bool                                        m_enabled;      // Main switch
        bool                                        m_scanOnStart;  // Scan always on start
        bool                                        m_hardware;     // Hardware inventory
        bool                                        m_os;           // OS inventory
        bool                                        m_network;      // Network inventory
        bool                                        m_packages;     // Installed packages inventory
        bool                                        m_ports;        // Opened ports inventory
        bool                                        m_portsAll;     // Scan only listening ports or all
        bool                                        m_processes;    // Running processes inventory
        bool                                        m_hotfixes;     // Windows hotfixes installed
        bool                                        m_stopping;
        bool                                        m_notify;
        std::unique_ptr<DBSync>                     m_spDBSync;
        std::condition_variable                     m_cv;
        std::mutex                                  m_mutex;
        std::unique_ptr<InvNormalizer>              m_spNormalizer;
        std::string                                 m_scanTime;

        void send_diff_message(const std::string& data);
        void send_dbsync_message(const std::string& data);
        void send_message(std::string data, const char queue_id);
        void log_config();

        void inventory_start();

        cJSON * dump();

        unsigned int interval;

        std::string dbPath;
        std::string normalizerConfigPath;
        std::string normalizerType;

        long sync_max_eps;            // Maximum events per second for synchronization messages.

};
