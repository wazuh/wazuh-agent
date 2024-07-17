#ifndef INVENTORY_H
#define INVENTORY_H

#include <string>
#include <cjson/cJSON.h>
#include "configuration.h"

class Inventory {
public:
    void *run();
    int setup(const Configuration & config);
    void stop();
    std::string command(const std::string & query);
    std::string name() const;
private:

    void wm_inv_send_diff_message(const std::string& data);
    void wm_inv_send_dbsync_message(const std::string& data);
    void wm_inv_send_message(std::string data, const char queue_id);
    void wm_inv_log_config();

    void inventory_start();
                        
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

