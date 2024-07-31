#ifndef INVENTORY_H
#define INVENTORY_H

#include <chrono>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <memory>
#include <string>
#include <memory>
#include <cjson/cJSON.h>
#include "sysInfoInterface.h"
#include "configuration.h"
#include "logging_helper.h"
#include "commonDefs.h"
#include "dbsync.hpp"
#include "inventoryNormalizer.h"

class Inventory {
public:

    Inventory();

    void *run();
    int setup(const Configuration & config);
    void stop();
    std::string command(const std::string & query);
    std::string name() const;

private:

    void init(const std::shared_ptr<ISysInfo>& spInfo,
                const std::string& dbPath,
                const std::string& normalizerConfigPath,
                const std::string& normalizerType);

    void destroy();
    void push(const std::string& data);

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
    void sync();
    void syncLoop(std::unique_lock<std::mutex>& lock);
    void syncAlgorithm();

    static void log(const modules_log_level_t, const std::string&);
    static void logError(const std::string&);

    std::shared_ptr<ISysInfo>                                               m_spInfo;
    std::function<void(const std::string&)>                                 m_reportDiffFunction;
    std::function<void(const std::string&)>                                 m_reportSyncFunction;
    std::chrono::seconds                                                    m_intervalValue;
    std::chrono::seconds                                                    m_currentIntervalValue;
    bool                                                                    m_scanOnStart;
    bool                                                                    m_hardware;
    bool                                                                    m_os;
    bool                                                                    m_network;
    bool                                                                    m_packages;
    bool                                                                    m_ports;
    bool                                                                    m_portsAll;
    bool                                                                    m_processes;
    bool                                                                    m_hotfixes;
    bool                                                                    m_stopping;
    bool                                                                    m_notify;
    std::unique_ptr<DBSync>                                                 m_spDBSync;
    std::condition_variable                                                 m_cv;
    std::mutex                                                              m_mutex;
    std::unique_ptr<InvNormalizer>                                          m_spNormalizer;
    std::string                                                             m_scanTime;
    std::chrono::seconds                                                    m_lastSyncMsg;

    void wm_inv_send_diff_message(const std::string& data);
    void wm_inv_send_dbsync_message(const std::string& data);
    void wm_inv_send_message(std::string data, const char queue_id);
    void wm_inv_log_config();

    void inventory_start();
    int inventory_sync_message(const char* data);


    cJSON * wm_inv_dump();

    unsigned int interval;

    std::string dbPath;
    std::string normalizerConfigPath;
    std::string normalizerType;

    pthread_cond_t inv_stop_condition;
    pthread_mutex_t inv_stop_mutex;
    bool need_shutdown_wait;
    pthread_mutex_t inv_reconnect_mutex;
    bool shutdown_process_started;

    bool enabled;                 // Main switch
    bool scan_on_start;           // Scan always on start
    bool hwinfo;                  // Hardware inventory
    bool netinfo;                 // Network inventory
    bool osinfo;                  // OS inventory
    bool programinfo;             // Installed packages inventory
    bool hotfixinfo;              // Windows hotfixes installed
    bool portsinfo;               // Opened ports inventory
    bool allports;                // Scan only listening ports or all
    bool procinfo;                // Running processes inventory

    long sync_max_eps;            // Maximum events per second for synchronization messages.

};

#endif // INVENTORY_H

