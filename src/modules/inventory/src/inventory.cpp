/*
 * Wazuh Inventory
 * Copyright (C) 2015, Wazuh Inc.
 * July 15, 2024.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 3) as published by the FSF - Free Software
 * Foundation.
 */

#include <iostream>
#include "debug_op.h"
#include "string_op.h"
#include "inventory.h"

using namespace std;

const int INVENTORY_MQ =  'd';
const int DBSYNC_MQ    = '5';

#define WM_INV_LOGTAG "modules:inventory" // Tag for log messages
#define WM_INVENTORY_DEFAULT_INTERVAL W_HOUR_SECONDS

void *Inventory::run() {
    cout << "+ [Inventory] is running" << endl;

    return NULL;
}

int Inventory::setup(const Configuration & config) {
    interval = WM_INVENTORY_DEFAULT_INTERVAL;
    dbPath = "default/path";
    normalizerConfigPath = "default/config/path";
    normalizerType = "normalizerType";

    enabled = true;
    scan_on_start = true;
    hwinfo = true;
    netinfo = true;
    osinfo = true;
    programinfo = true;
    hotfixinfo = true;
    portsinfo = true;
    allports = true;
    procinfo = true;
    return 0;
}

void Inventory::stop() {
    cout << "- [Inventory] stopped" << endl;
}

string Inventory::command(const string & query) {
    cout << "  [Inventory] query: " << query << endl;
    return "OK";
}

string Inventory::name() const {
    return "inventory";
}

void Inventory::wm_inv_send_diff_message(const string& data) {
    wm_inv_send_message(data, INVENTORY_MQ);
}

void Inventory::wm_inv_send_dbsync_message(const string& data) {
    wm_inv_send_message(data, DBSYNC_MQ);
}

void Inventory::wm_inv_send_message(string data, const char queue_id) {
    /*if (!is_shutdown_process_started()) {
        const int eps = 1000000/inventory_sync_max_eps;
        if (wm_sendmsg_ex(eps, queue_fd, data, WM_INV_LOCATION, queue_id, &is_shutdown_process_started) < 0) {

            mterror(WM_INV_LOGTAG, "Unable to send message to '%s' (wazuh-agentd might be down). Attempting to reconnect.", DEFAULTQUEUE);
    
            // Since this method is beign called by multiple threads it's necessary this particular portion of code
            // to be mutually exclusive. When one thread is successfully reconnected, the other ones will make use of it.
            w_mutex_lock(&inv_reconnect_mutex);
            if (!is_shutdown_process_started() && wm_sendmsg_ex(eps, queue_fd, data, WM_INV_LOCATION, queue_id, &is_shutdown_process_started) < 0) {
                if (queue_fd = MQReconnectPredicated(DEFAULTQUEUE, &is_shutdown_process_started), 0 <= queue_fd) {
                    mtinfo(WM_INV_LOGTAG, "Successfully reconnected to '%s'", DEFAULTQUEUE);
                    if (wm_sendmsg_ex(eps, queue_fd, data, WM_INV_LOCATION, queue_id, &is_shutdown_process_started) < 0) {
                        mterror(WM_INV_LOGTAG, "Unable to send message to '%s' after a successfull reconnection...", DEFAULTQUEUE);
                    }
                }
            }
            w_mutex_unlock(&inv_reconnect_mutex);
        }
    }*/
   cout << "wm_inv_send_message -> Queue id: " << queue_id << " -> Data: " << data << endl;
}

void Inventory::wm_inv_log_config()
{
    cJSON * config_json = wm_inv_dump();
    if (config_json) {
        char * config_str = cJSON_PrintUnformatted(config_json);
        if (config_str) {
            cout << WM_INV_LOGTAG << config_str << endl;
            cJSON_free(config_str);
        }
        cJSON_Delete(config_json);
    }
}

cJSON * Inventory::wm_inv_dump() {
    cJSON *root = cJSON_CreateObject();
    cJSON *wm_inv = cJSON_CreateObject();

    // System provider values
    if (enabled) cJSON_AddStringToObject(wm_inv,"disabled","no"); else cJSON_AddStringToObject(wm_inv,"disabled","yes");
    if (scan_on_start) cJSON_AddStringToObject(wm_inv,"scan-on-start","yes"); else cJSON_AddStringToObject(wm_inv,"scan-on-start","no");
    cJSON_AddNumberToObject(wm_inv,"interval",interval);
    if (netinfo) cJSON_AddStringToObject(wm_inv,"network","yes"); else cJSON_AddStringToObject(wm_inv,"network","no");
    if (osinfo) cJSON_AddStringToObject(wm_inv,"os","yes"); else cJSON_AddStringToObject(wm_inv,"os","no");
    if (hwinfo) cJSON_AddStringToObject(wm_inv,"hardware","yes"); else cJSON_AddStringToObject(wm_inv,"hardware","no");
    if (programinfo) cJSON_AddStringToObject(wm_inv,"packages","yes"); else cJSON_AddStringToObject(wm_inv,"packages","no");
    if (portsinfo) cJSON_AddStringToObject(wm_inv,"ports","yes"); else cJSON_AddStringToObject(wm_inv,"ports","no");
    if (allports) cJSON_AddStringToObject(wm_inv,"ports_all","yes"); else cJSON_AddStringToObject(wm_inv,"ports_all","no");
    if (procinfo) cJSON_AddStringToObject(wm_inv,"processes","yes"); else cJSON_AddStringToObject(wm_inv,"processes","no");
#ifdef WIN32
    if (hotfixinfo) cJSON_AddStringToObject(wm_inv,"hotfixes","yes"); else cJSON_AddStringToObject(wm_inv,"hotfixes","no");
#endif
    // Database synchronization values
    cJSON_AddNumberToObject(wm_inv,"sync_max_eps",sync_max_eps);

    cJSON_AddItemToObject(root,"inventory",wm_inv);

    return root;
}