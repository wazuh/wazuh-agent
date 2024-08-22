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
#include <cjson/cJSON.h>
#include "shared.h"
#include "defs.h"
#include "logging_helper.h"
#include "string_op.h"
#include "pthreads_op.h"

#include "inventory.hpp"
#include "sysInfo.hpp"

using namespace std;

const int INVENTORY_MQ =  'd';
const int DBSYNC_MQ    = '5';

#define INFINITE_OPENQ_ATTEMPTS 0

#define INV_LOGTAG "modules:inventory" // Tag for log messages

void *Inventory::start() {

    if (!m_enabled) {
        log(LOG_INFO, "Module disabled. Exiting...");
        pthread_exit(NULL);
    }

    log(LOG_INFO, "Starting inventory.");

    log_config();

    run();

    log(LOG_INFO, "Module finished.");

    return NULL;

}

int Inventory::setup(const Configuration & config) {

    const InventoryConfig& inventoryConfig = config.getInventoryConfig();

    m_enabled = inventoryConfig.enabled;
    m_intervalValue = std::chrono::seconds{inventoryConfig.interval};
    m_scanOnStart = inventoryConfig.scanOnStart;
    m_hardware = inventoryConfig.hardware;
    m_os = inventoryConfig.os;
    m_network = inventoryConfig.network;
    m_packages = inventoryConfig.packages;
    m_ports = inventoryConfig.ports;
    m_portsAll = inventoryConfig.portsAll;
    m_processes = inventoryConfig.processes;
    m_hotfixes = inventoryConfig.hotfixes;

    m_notify = true;

    return 0;
}

void Inventory::stop() {
    cout << "- [Inventory] stopped" << endl;
    Inventory::instance().destroy();
}

string Inventory::command(const string & query) {
    cout << "  [Inventory] query: " << query << endl;
    return "OK";
}

string Inventory::name() const {
    return "Inventory";
}

void Inventory::send_diff_message(const string& data) {
    send_message(data, INVENTORY_MQ);
}

void Inventory::send_dbsync_message(const string& data) {
    send_message(data, DBSYNC_MQ);
}

void Inventory::send_message(string data, const char queue_id) {
    /*if (!is_shutdown_process_started()) {
        const int eps = 1000000/sync_max_eps;
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
}

void Inventory::log_config()
{
    cJSON * config_json = dump();
    if (config_json) {
        char * config_str = cJSON_PrintUnformatted(config_json);
        if (config_str) {
            log(LOG_DEBUG, config_str);
            cJSON_free(config_str);
        }
        cJSON_Delete(config_json);
    }
}

cJSON * Inventory::dump() {

    cJSON *root = cJSON_CreateObject();
    cJSON *wm_inv = cJSON_CreateObject();

    // System provider values
    if (m_enabled) cJSON_AddStringToObject(wm_inv,"disabled","no"); else cJSON_AddStringToObject(wm_inv,"disabled","yes");
    if (m_scanOnStart) cJSON_AddStringToObject(wm_inv,"scan-on-start","yes"); else cJSON_AddStringToObject(wm_inv,"scan-on-start","no");
    cJSON_AddNumberToObject(wm_inv,"interval",interval);
    if (m_network) cJSON_AddStringToObject(wm_inv,"network","yes"); else cJSON_AddStringToObject(wm_inv,"network","no");
    if (m_os) cJSON_AddStringToObject(wm_inv,"os","yes"); else cJSON_AddStringToObject(wm_inv,"os","no");
    if (m_hardware) cJSON_AddStringToObject(wm_inv,"hardware","yes"); else cJSON_AddStringToObject(wm_inv,"hardware","no");
    if (m_packages) cJSON_AddStringToObject(wm_inv,"packages","yes"); else cJSON_AddStringToObject(wm_inv,"packages","no");
    if (m_ports) cJSON_AddStringToObject(wm_inv,"ports","yes"); else cJSON_AddStringToObject(wm_inv,"ports","no");
    if (m_portsAll) cJSON_AddStringToObject(wm_inv,"ports_all","yes"); else cJSON_AddStringToObject(wm_inv,"ports_all","no");
    if (m_processes) cJSON_AddStringToObject(wm_inv,"processes","yes"); else cJSON_AddStringToObject(wm_inv,"processes","no");
#ifdef WIN32
    if (m_hotfixes) cJSON_AddStringToObject(wm_inv,"hotfixes","yes"); else cJSON_AddStringToObject(wm_inv,"hotfixes","no");
#endif
    // Database synchronization values
    cJSON_AddNumberToObject(wm_inv,"sync_max_eps",sync_max_eps);

    cJSON_AddItemToObject(root,"inventory",wm_inv);

    return root;
}


void Inventory::run()
{
    DBSync::initialize(logError);

    try
    {
        Inventory::instance().init(std::make_shared<SysInfo>(),
                                      dbPath,
                                      normalizerConfigPath,
                                      normalizerType);
    }
    catch (const std::exception& ex)
    {
        logError(ex.what());
    }
}

void Inventory::log(const modules_log_level_t level, const std::string& log)
{
    taggedLogFunction(level, log.c_str(), INV_LOGTAG);
}

void Inventory::logError(const std::string& log)
{
    taggedLogFunction(LOG_ERROR, log.c_str(), INV_LOGTAG);
}