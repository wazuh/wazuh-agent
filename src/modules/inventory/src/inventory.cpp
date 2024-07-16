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

#include "inventory.h"
#include <iostream>

using namespace std;
const int INVENTORY_MQ =  'd';
const int DBSYNC_MQ    = '5';

void *Inventory::run() {
    cout << "+ [Inventory] is running" << endl;

    return NULL;
}

int Inventory::setup(const Configuration & config) {
    inverval = 3600;
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
