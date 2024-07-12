/*
 * Wazuh INVENTORY
 * Copyright (C) 2015, Wazuh Inc.
 * November 11, 2021.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */
#include <stdlib.h>
#include "../../wmodules_def.h"
#include "wmodules.h"
#include "wm_inventory.h"
#include "inventory.h"
#include "sym_load.h"
#include "defs.h"
#include "mq_op.h"
#include "headers/logging_helper.h"
#include "commonDefs.h"

#ifndef CLIENT
#include "router.h"
#include "utils/flatbuffers/include/inventory_synchronization_schema.h"
#include "utils/flatbuffers/include/inventory_deltas_schema.h"
#include "agent_messages_adapter.h"
#endif // CLIENT

#ifdef WIN32
static DWORD WINAPI wm_inv_main(void *arg);         // Module main function. It won't return
#else
static void* wm_inv_main(wm_inv_t *inv);        // Module main function. It won't return
#endif
static void wm_inv_destroy(wm_inv_t *data);      // Destroy data
static void wm_inv_stop(wm_inv_t *inv);         // Module stopper
const char *WM_INV_LOCATION = "inventory";   // Location field for event sending
cJSON *wm_inv_dump(const wm_inv_t *inv);
int wm_inv_message(const char *data);
pthread_cond_t inv_stop_condition = PTHREAD_COND_INITIALIZER;
pthread_mutex_t inv_stop_mutex = PTHREAD_MUTEX_INITIALIZER;
bool need_shutdown_wait = false;
pthread_mutex_t inv_reconnect_mutex = PTHREAD_MUTEX_INITIALIZER;
bool shutdown_process_started = false;

const wm_context WM_INV_CONTEXT = {
    .name = "inventory",
    .start = (wm_routine)wm_inv_main,
    .destroy = (void(*)(void *))wm_inv_destroy,
    .dump = (cJSON * (*)(const void *))wm_inv_dump,
    .sync = (int(*)(const char*))wm_sync_message,
    .stop = (void(*)(void *))wm_inv_stop,
    .query = NULL,
};

void *inventory_module = NULL;
inventory_start_func inventory_start_ptr = NULL;
inventory_stop_func inventory_stop_ptr = NULL;
inventory_sync_message_func inventory_sync_message_ptr = NULL;

#ifndef CLIENT
void *router_module_ptr = NULL;
router_provider_create_func router_provider_create_func_ptr = NULL;
router_provider_send_fb_func router_provider_send_fb_func_ptr = NULL;
ROUTER_PROVIDER_HANDLE rsync_handle = NULL;
ROUTER_PROVIDER_HANDLE inventory_handle = NULL;
int disable_manager_scan = 1;
#endif // CLIENT

long inventory_sync_max_eps = 10;    // Database synchronization number of events per second (default value)
int queue_fd = 0;                       // Output queue file descriptor

static bool is_shutdown_process_started() {
    bool ret_val = shutdown_process_started;
    return ret_val;
}

static void wm_inv_send_message(const void* data, const char queue_id) {
    if (!is_shutdown_process_started()) {
        const int eps = 1000000/inventory_sync_max_eps;
        if (wm_sendmsg_ex(eps, queue_fd, data, WM_INV_LOCATION, queue_id, &is_shutdown_process_started) < 0) {
    #ifdef CLIENT
            mterror(WM_INV_LOGTAG, "Unable to send message to '%s' (wazuh-agentd might be down). Attempting to reconnect.", DEFAULTQUEUE);
    #else
            mterror(WM_INV_LOGTAG, "Unable to send message to '%s' (wazuh-analysisd might be down). Attempting to reconnect.", DEFAULTQUEUE);
    #endif
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
    }
}

static void wm_inv_send_diff_message(const void* data) {
    wm_inv_send_message(data, INVENTORY_MQ);
#ifndef CLIENT
    if(!disable_manager_scan)
    {
        char* msg_to_send = adapt_delta_message(data, "localhost", "000", "127.0.0.1", NULL);
        if (msg_to_send && router_provider_send_fb_func_ptr) {
            router_provider_send_fb_func_ptr(inventory_handle, msg_to_send, inventory_deltas_SCHEMA);
        }
        cJSON_free(msg_to_send);
    }
#endif // CLIENT
}

static void wm_inv_send_dbsync_message(const void* data) {
    wm_inv_send_message(data, DBSYNC_MQ);
#ifndef CLIENT
    if(!disable_manager_scan)
    {
        char* msg_to_send = adapt_sync_message(data, "localhost", "000", "127.0.0.1", NULL);
        if (msg_to_send && router_provider_send_fb_func_ptr) {
            router_provider_send_fb_func_ptr(rsync_handle, msg_to_send, inventory_synchronization_SCHEMA);
        }
        cJSON_free(msg_to_send);
    }
#endif // CLIENT
}

static void wm_inv_log_config(wm_inv_t *inv)
{
    cJSON * config_json = wm_inv_dump(inv);
    if (config_json) {
        char * config_str = cJSON_PrintUnformatted(config_json);
        if (config_str) {
            mtdebug1(WM_INV_LOGTAG, "%s", config_str);
            cJSON_free(config_str);
        }
        cJSON_Delete(config_json);
    }
}

#ifdef WIN32
DWORD WINAPI wm_inv_main(void *arg) {
    wm_inv_t *inv = (wm_inv_t *)arg;
#else
void* wm_inv_main(wm_inv_t *inv) {
#endif
    w_cond_init(&inv_stop_condition, NULL);
    w_mutex_init(&inv_stop_mutex, NULL);
    w_mutex_init(&inv_reconnect_mutex, NULL);

    if (!inv->flags.enabled) {
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
        inventory_start_ptr = so_get_function_sym(inventory_module, "inventory_start");
        inventory_stop_ptr = so_get_function_sym(inventory_module, "inventory_stop");
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
#ifndef CLIENT
        // Load router module only for manager if is enabled
        disable_manager_scan = getDefine_Int("vulnerability-detection", "disable_scan_manager", 0, 1);
        if (router_module_ptr = so_get_module_handle("router"), router_module_ptr) {
                router_provider_create_func_ptr = so_get_function_sym(router_module_ptr, "router_provider_create");
                router_provider_send_fb_func_ptr = so_get_function_sym(router_module_ptr, "router_provider_send_fb");
                if (router_provider_create_func_ptr && router_provider_send_fb_func_ptr) {
                    mtdebug1(WM_INV_LOGTAG, "Router module loaded.");
                } else {
                    mwarn("Failed to load methods from router module.");
                }
            } else {
                mwarn("Failed to load router module.");
            }
#endif // CLIENT
    } else {
#ifdef __hpux
        mtinfo(WM_INV_LOGTAG, "Not supported in HP-UX.");
#else
        mterror(WM_INV_LOGTAG, "Can't load inventory.");
#endif
        pthread_exit(NULL);
    }
    if (inventory_start_ptr) {
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
#ifndef CLIENT
        // Router providers initialization
        if (router_provider_create_func_ptr){
            if(inventory_handle = router_provider_create_func_ptr("deltas-inventory", true), !inventory_handle) {
                mdebug2("Failed to create router handle for 'inventory'.");
            }

            if (rsync_handle = router_provider_create_func_ptr("rsync-inventory", true), !rsync_handle) {
                mdebug2("Failed to create router handle for 'rsync'.");
            }
        }
#endif // CLIENT
        inventory_start_ptr(inv->interval,
                               wm_inv_send_diff_message,
                               wm_inv_send_dbsync_message,
                               taggedLogFunction,
                               INVENTORY_DB_DISK_PATH,
                               INVENTORY_NORM_CONFIG_DISK_PATH,
                               INVENTORY_NORM_TYPE,
                               inv->flags.scan_on_start,
                               inv->flags.hwinfo,
                               inv->flags.osinfo,
                               inv->flags.netinfo,
                               inv->flags.programinfo,
                               inv->flags.portsinfo,
                               inv->flags.allports,
                               inv->flags.procinfo,
                               inv->flags.hotfixinfo);
    } else {
        mterror(WM_INV_LOGTAG, "Can't get inventory_start_ptr.");
        pthread_exit(NULL);
    }
    inventory_sync_message_ptr = NULL;
    inventory_start_ptr = NULL;
    inventory_stop_ptr = NULL;

    if (queue_fd) {
        close(queue_fd);
        queue_fd = 0;
    }
    so_free_library(inventory_module);
#ifndef CLIENT
    so_free_library(router_module_ptr);
    router_module_ptr = NULL;
#endif // CLIENT
    inventory_module = NULL;
    mtinfo(WM_INV_LOGTAG, "Module finished.");
    w_mutex_lock(&inv_stop_mutex);
    w_cond_signal(&inv_stop_condition);
    w_mutex_unlock(&inv_stop_mutex);
    return 0;
}

void wm_inv_destroy(wm_inv_t *data) {
    free(data);
}

void wm_inv_stop(__attribute__((unused))wm_inv_t *data) {
    mtinfo(WM_INV_LOGTAG, "Stop received for Inventory.");
    inventory_sync_message_ptr = NULL;
    if (inventory_stop_ptr){
        shutdown_process_started = true;
        inventory_stop_ptr();
    }
    w_mutex_lock(&inv_stop_mutex);
    if (need_shutdown_wait) {
        w_cond_wait(&inv_stop_condition, &inv_stop_mutex);
    }
    w_mutex_unlock(&inv_stop_mutex);

    w_cond_destroy(&inv_stop_condition);
    w_mutex_destroy(&inv_stop_mutex);
    w_mutex_destroy(&inv_reconnect_mutex);
}

cJSON *wm_inv_dump(const wm_inv_t *inv) {
    cJSON *root = cJSON_CreateObject();
    cJSON *wm_inv = cJSON_CreateObject();

    // System provider values
    if (inv->flags.enabled) cJSON_AddStringToObject(wm_inv,"disabled","no"); else cJSON_AddStringToObject(wm_inv,"disabled","yes");
    if (inv->flags.scan_on_start) cJSON_AddStringToObject(wm_inv,"scan-on-start","yes"); else cJSON_AddStringToObject(wm_inv,"scan-on-start","no");
    cJSON_AddNumberToObject(wm_inv,"interval",inv->interval);
    if (inv->flags.netinfo) cJSON_AddStringToObject(wm_inv,"network","yes"); else cJSON_AddStringToObject(wm_inv,"network","no");
    if (inv->flags.osinfo) cJSON_AddStringToObject(wm_inv,"os","yes"); else cJSON_AddStringToObject(wm_inv,"os","no");
    if (inv->flags.hwinfo) cJSON_AddStringToObject(wm_inv,"hardware","yes"); else cJSON_AddStringToObject(wm_inv,"hardware","no");
    if (inv->flags.programinfo) cJSON_AddStringToObject(wm_inv,"packages","yes"); else cJSON_AddStringToObject(wm_inv,"packages","no");
    if (inv->flags.portsinfo) cJSON_AddStringToObject(wm_inv,"ports","yes"); else cJSON_AddStringToObject(wm_inv,"ports","no");
    if (inv->flags.allports) cJSON_AddStringToObject(wm_inv,"ports_all","yes"); else cJSON_AddStringToObject(wm_inv,"ports_all","no");
    if (inv->flags.procinfo) cJSON_AddStringToObject(wm_inv,"processes","yes"); else cJSON_AddStringToObject(wm_inv,"processes","no");
#ifdef WIN32
    if (inv->flags.hotfixinfo) cJSON_AddStringToObject(wm_inv,"hotfixes","yes"); else cJSON_AddStringToObject(wm_inv,"hotfixes","no");
#endif
    // Database synchronization values
    cJSON_AddNumberToObject(wm_inv,"sync_max_eps",inv->sync.sync_max_eps);

    cJSON_AddItemToObject(root,"inventory",wm_inv);

    return root;
}

int wm_sync_message(const char *data)
{
    int ret_val = 0;

    if (inventory_sync_message_ptr) {
        ret_val = inventory_sync_message_ptr(data);
    }

    return ret_val;
}
