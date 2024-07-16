#ifndef INVENTORY_H
#define INVENTORY_H

#include <string>
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

    unsigned int inverval;
    
    std::string dbPath;
    std::string normalizerConfigPath;
    std::string normalizerType;

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
};

#endif // INVENTORY_H

