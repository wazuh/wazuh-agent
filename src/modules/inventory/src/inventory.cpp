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
#include "shared.h"
#include "defs.h"
#include "logging_helper.h"
#include "string_op.h"
#include "pthreads_op.h"

#include "inventory.h"
#include "sysInfo.hpp"

using namespace std;

const int INVENTORY_MQ =  'd';
const int DBSYNC_MQ    = '5';

#define INFINITE_OPENQ_ATTEMPTS 0

#define WM_INV_LOGTAG "modules:inventory" // Tag for log messages
#define WM_INVENTORY_DEFAULT_INTERVAL W_HOUR_SECONDS

void *Inventory::run() {
    w_cond_init(&inv_stop_condition, NULL);
    w_mutex_init(&inv_stop_mutex, NULL);
    w_mutex_init(&inv_reconnect_mutex, NULL);

    if (!m_enabled) {
        log(LOG_INFO, "Module disabled. Exiting...");
        pthread_exit(NULL);
    }

    log(LOG_DEBUG, "Starting inventory.");
    w_mutex_lock(&inv_stop_mutex);
    need_shutdown_wait = true;
    w_mutex_unlock(&inv_stop_mutex);

    // else: if max_eps is 0 (from configuration) let's use the default max_eps value (10)
    wm_inv_log_config();

    inventory_start();

    log(LOG_INFO, "Module finished.");
    w_mutex_lock(&inv_stop_mutex);
    w_cond_signal(&inv_stop_condition);
    w_mutex_unlock(&inv_stop_mutex);
    return NULL;

}

int Inventory::setup(const Configuration & config) {

    inv_stop_condition = PTHREAD_COND_INITIALIZER;
    inv_stop_mutex = PTHREAD_MUTEX_INITIALIZER;
    need_shutdown_wait = false;
    inv_reconnect_mutex = PTHREAD_MUTEX_INITIALIZER;
    shutdown_process_started = false;

    dbPath = INVENTORY_DB_DISK_PATH;
    normalizerConfigPath = INVENTORY_NORM_CONFIG_DISK_PATH;
    normalizerType = INVENTORY_NORM_TYPE;

    sync_max_eps = 10;      // Database synchronization number of events per second (default value)

    m_intervalValue = std::chrono::seconds{WM_INVENTORY_DEFAULT_INTERVAL};

    m_enabled = true;

    m_scanOnStart = true;
    m_hardware = true;
    m_os = true;
    m_network = true;
    m_packages = true;
    m_ports = true;
    m_portsAll = true;
    m_processes = true;
    m_hotfixes = true;
    m_notify = false;

    m_currentIntervalValue = m_intervalValue;

    return 0;
}

void Inventory::stop() {
    cout << "- [Inventory] stopped" << endl;
    Inventory::destroy();
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
   cout << "wm_inv_send_message -> Queue id: " << queue_id << " -> Data: " << data << endl;
}

void Inventory::wm_inv_log_config()
{
    cJSON * config_json = wm_inv_dump();
    if (config_json) {
        char * config_str = cJSON_PrintUnformatted(config_json);
        if (config_str) {
            log(LOG_DEBUG, config_str);
            cJSON_free(config_str);
        }
        cJSON_Delete(config_json);
    }
}

cJSON * Inventory::wm_inv_dump() {
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


void Inventory::inventory_start()
{

    DBSync::initialize(logError);

    try
    {
        init(std::make_shared<SysInfo>(),
                                      dbPath,
                                      normalizerConfigPath,
                                      normalizerType);
    }
    catch (const std::exception& ex)
    {
        logError(ex.what());
    }
}

int Inventory::inventory_sync_message(const char* data)
{
    int ret{-1};

    try
    {
        push(data);
        ret = 0;
    }
    catch (...)
    {
    }

    return ret;
}

void Inventory::log1(const modules_log_level_t level, const std::string& log, const char* file, int line)
{
    std::string tag{};

    tag += WM_INV_LOGTAG;
    tag += " ";
    tag += file;
    tag += " (";
    tag += std::to_string(line);
    tag += ") ";
    taggedLogFunction(level, log.c_str(), tag.c_str());
}

void Inventory::logError(const std::string& log)
{
    taggedLogFunction(LOG_ERROR, log.c_str(), WM_INV_LOGTAG);
}