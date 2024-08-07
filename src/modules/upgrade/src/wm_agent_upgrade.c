/*
 * Wazuh Module for Agent Upgrading
 * Copyright (C) 2015, Wazuh Inc.
 * July 3, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifdef WAZUH_UNIT_TESTING
// Remove static qualifier when unit testing
#define STATIC
#else
#define STATIC static
#endif

#include "wazuh_modules/wmodules.h"
#include "os_net/os_net.h"

#include "agent/wm_agent_upgrade_agent.h"

/**
 * Module main function. It won't return
 * */
#ifdef WIN32
STATIC DWORD WINAPI wm_agent_upgrade_main(void *arg);
#else
STATIC void* wm_agent_upgrade_main(wm_agent_upgrade* upgrade_config);
#endif
STATIC void wm_agent_upgrade_destroy(wm_agent_upgrade* upgrade_config);
STATIC cJSON *wm_agent_upgrade_dump(const wm_agent_upgrade* upgrade_config);

/* Context definition */
const wm_context WM_AGENT_UPGRADE_CONTEXT = {
    .name = AGENT_UPGRADE_WM_NAME,
    .start = (wm_routine)wm_agent_upgrade_main,
    .destroy = (void(*)(void *))wm_agent_upgrade_destroy,
    .dump = (cJSON * (*)(const void *))wm_agent_upgrade_dump,
    .sync = NULL,
    .stop = NULL,
    .query = NULL,
};

#ifdef WIN32
STATIC DWORD WINAPI wm_agent_upgrade_main(void *arg) {
    wm_agent_upgrade* upgrade_config = (wm_agent_upgrade *)arg;
#else
STATIC void *wm_agent_upgrade_main(wm_agent_upgrade* upgrade_config) {
#endif
wm_agent_upgrade_start_agent_module(&upgrade_config->agent_config, upgrade_config->enabled);


#ifdef WIN32
    return 0;
#else
    return NULL;
#endif
}

STATIC void wm_agent_upgrade_destroy(wm_agent_upgrade* upgrade_config) {
    mtinfo(WM_AGENT_UPGRADE_LOGTAG, WM_UPGRADE_MODULE_FINISHED);
    os_free(upgrade_config);
}

STATIC cJSON *wm_agent_upgrade_dump(const wm_agent_upgrade* upgrade_config){
    cJSON *root = cJSON_CreateObject();
    cJSON *wm_info = cJSON_CreateObject();

    if (upgrade_config->enabled) {
        cJSON_AddStringToObject(wm_info,"enabled","yes");
    } else {
        cJSON_AddStringToObject(wm_info,"enabled","no");
    }
    if (upgrade_config->agent_config.enable_ca_verification) {
        cJSON_AddStringToObject(wm_info,"ca_verification","yes");
    } else {
        cJSON_AddStringToObject(wm_info,"ca_verification","no");
    }
    if (wcom_ca_store) {
        cJSON *calist = cJSON_CreateArray();
        for (int i=0; wcom_ca_store[i]; i++) {
            cJSON_AddItemToArray(calist,cJSON_CreateString(wcom_ca_store[i]));
        }
        cJSON_AddItemToObject(wm_info,"ca_store",calist);
    }
    cJSON_AddItemToObject(root,"agent-upgrade",wm_info);
    return root;
}
