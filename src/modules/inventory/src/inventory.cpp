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

void Inventory::Inventory(){
    inv_stop_condition = PTHREAD_COND_INITIALIZER;
    inv_stop_mutex = PTHREAD_MUTEX_INITIALIZER;
    need_shutdown_wait = false;
    inv_reconnect_mutex = PTHREAD_MUTEX_INITIALIZER;
    shutdown_process_started = false;

    dbPath = INVENTORY_DB_DISK_PATH;
    normalizerConfigPath = INVENTORY_NORM_CONFIG_DISK_PATH;
    normalizerType = INVENTORY_NORM_TYPE;
}

void Inventory::~Inventory(){
    cout << "Inventory destroyed!" << endl;
}

void *Inventory::run() {
    w_cond_init(&inv_stop_condition, NULL);
    w_mutex_init(&inv_stop_mutex, NULL);
    w_mutex_init(&inv_reconnect_mutex, NULL);

    if (!enabled) {
        mtinfo(WM_INV_LOGTAG, "Module disabled. Exiting...");
        pthread_exit(NULL);
    }

    #ifndef WIN32
    // Connect to socket
    queue_fd = StartMQ(DEFAULTQUEUE, WRITE, INFINITE_OPENQ_ATTEMPTS);

    if (queue_fd < 0) {
        mterror(WM_INV_LOGTAG, "Can't connect to queue.");
        pthread_exit(NULL);
    }
    #endif

    if (inventory_module = so_get_module_handle("inventory"), inventory_module)
    {
        inventory_sync_message_ptr = so_get_function_sym(inventory_module, "inventory_sync_message");

        void* rsync_module = NULL;
        if(rsync_module = so_check_module_loaded("rsync"), rsync_module) {
            rsync_initialize_full_log_func rsync_initialize_log_function_ptr = so_get_function_sym(rsync_module, "rsync_initialize_full_log_function");
            if(rsync_initialize_log_function_ptr) {
                rsync_initialize_log_function_ptr(mtLoggingFunctionsWrapper);
            }
            // Even when the RTLD_NOLOAD flag was used for dlopen(), we need a matching call to dlclose()
#ifndef WIN32
            so_free_library(rsync_module);
#endif
        }
    } else {
#ifdef __hpux
        mtinfo(WM_INV_LOGTAG, "Not supported in HP-UX.");
#else
        mterror(WM_INV_LOGTAG, "Can't load inventory.");
#endif
        pthread_exit(NULL);
    }

    mtdebug1(WM_INV_LOGTAG, "Starting inventory.");
    w_mutex_lock(&inv_stop_mutex);
    need_shutdown_wait = true;
    w_mutex_unlock(&inv_stop_mutex);
    const long max_eps = inv->sync.sync_max_eps;
    if (0 != max_eps) {
        inventory_sync_max_eps = max_eps;
    }
    // else: if max_eps is 0 (from configuration) let's use the default max_eps value (10)
    wm_inv_log_config(inv);

    inventory_start();

    inventory_sync_message_ptr = NULL;
    inventory_start_ptr = NULL;
    inventory_stop_ptr = NULL;

    if (queue_fd) {
        close(queue_fd);
        queue_fd = 0;
    }
    so_free_library(inventory_module);

    inventory_module = NULL;
    mtinfo(WM_INV_LOGTAG, "Module finished.");
    w_mutex_lock(&inv_stop_mutex);
    w_cond_signal(&inv_stop_condition);
    w_mutex_unlock(&inv_stop_mutex);
    return 0;

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
    Inventory::instance().destroy();
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


void Inventory::inventory_start()
{
    std::function<void(const std::string&)> callbackDiffWrapper
    {
        [callbackDiff](const std::string & data)
        {
            callbackDiff(data.c_str());
        }
    };

    std::function<void(const std::string&)> callbackSyncWrapper
    {
        [callbackSync](const std::string & data)
        {
            callbackSync(data.c_str());
        }
    };

    std::function<void(const modules_log_level_t, const std::string&)> callbackLogWrapper
    {
        [callbackLog](const modules_log_level_t level, const std::string & data)
        {
            callbackLog(level, data.c_str(), WM_INV_LOGTAG);
        }
    };

    std::function<void(const std::string&)> callbackErrorLogWrapper
    {
        [callbackLog](const std::string & data)
        {
            callbackLog(LOG_ERROR, data.c_str(), WM_INV_LOGTAG);
        }
    };

    DBSync::initialize(callbackErrorLogWrapper);

    try
    {
        Inventory::instance().init(std::make_shared<SysInfo>(),
                                      std::move(callbackDiffWrapper),
                                      std::move(callbackSyncWrapper),
                                      std::move(callbackLogWrapper),
                                      dbPath,
                                      normalizerConfigPath,
                                      normalizerType,
                                      inverval,
                                      scanOnStart,
                                      hardware,
                                      os,
                                      network,
                                      packages,
                                      ports,
                                      portsAll,
                                      processes,
                                      hotfixes);
    }
    catch (const std::exception& ex)
    {
        callbackErrorLogWrapper(ex.what());
    }
}

int Inventory::inventory_sync_message(const char* data)
{
    int ret{-1};

    try
    {
        Inventory::instance().push(data);
        ret = 0;
    }
    catch (...)
    {
    }

    return ret;
}
