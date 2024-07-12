/*
 * Wazuh Module for System inventory
 * Copyright (C) 2015, Wazuh Inc.
 * November 17, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "wmodules_def.h"
#include "../os_xml/os_xml.h"

#ifndef WM_INVENTORY
#define WM_INVENTORY

extern const wm_context WM_INV_CONTEXT;     // Context

#define WM_INV_LOGTAG ARGV0 ":inventory" // Tag for log messages
#define WM_INVENTORY_DEFAULT_INTERVAL W_HOUR_SECONDS

typedef struct wm_inv_flags_t {
    unsigned int enabled:1;                 // Main switch
    unsigned int scan_on_start:1;           // Scan always on start
    unsigned int hwinfo:1;                  // Hardware inventory
    unsigned int netinfo:1;                 // Network inventory
    unsigned int osinfo:1;                  // OS inventory
    unsigned int programinfo:1;             // Installed packages inventory
    unsigned int hotfixinfo:1;              // Windows hotfixes installed
    unsigned int portsinfo:1;               // Opened ports inventory
    unsigned int allports:1;                // Scan only listening ports or all
    unsigned int procinfo:1;                // Running processes inventory
} wm_inv_flags_t;

typedef struct wm_inv_state_t {
    time_t next_time;                       // Absolute time for next scan
} wm_inv_state_t;

typedef struct wm_inv_db_sync_flags_t {
    long sync_max_eps;                      // Maximum events per second for synchronization messages.
} wm_inv_db_sync_flags_t;

typedef struct wm_inv_t {
    unsigned int interval;                  // Time interval between cycles (seconds)
    wm_inv_flags_t flags;                   // Flag bitfield
    wm_inv_state_t state;                   // Running state
    wm_inv_db_sync_flags_t sync;            // Database synchronization value
} wm_inv_t;

// Parse XML configuration
int wm_inventory_read(const OS_XML *xml, XML_NODE node, wmodule *module);

#endif
