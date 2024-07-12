/*
 * Wazuh Inventory
 * Copyright (C) 2015, Wazuh Inc.
 * November 15, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */


#ifndef _INVENTORY_H
#define _INVENTORY_H

// Define EXPORTED for any platform
#ifdef _WIN32
#ifdef WIN_EXPORT
#define EXPORTED __declspec(dllexport)
#else
#define EXPORTED __declspec(dllimport)
#endif
#elif __GNUC__ >= 4
#define EXPORTED __attribute__((visibility("default")))
#else
#define EXPORTED
#endif

#include <stdbool.h>
#include "commonDefs.h"
#ifdef __cplusplus
extern "C" {
#endif
#include "logging_helper.h"

typedef void((*log_callback_t)(const modules_log_level_t level, const char* log, const char* tag));

typedef void((*send_data_callback_t)(const void* buffer));

EXPORTED void inventory_start(const unsigned int inverval,
                                 send_data_callback_t callbackDiff,
                                 send_data_callback_t callbackSync,
                                 log_callback_t callbackLog,
                                 const char* dbPath,
                                 const char* normalizerConfigPath,
                                 const char* normalizerType,
                                 const bool scanOnStart,
                                 const bool hardware,
                                 const bool os,
                                 const bool network,
                                 const bool packages,
                                 const bool ports,
                                 const bool portsAll,
                                 const bool processes,
                                 const bool hotfixes);

EXPORTED void inventory_stop();

EXPORTED int inventory_sync_message(const char* data);



#ifdef __cplusplus
}
#endif

typedef void(*inventory_start_func)(const unsigned int inverval,
                                       send_data_callback_t callbackDiff,
                                       send_data_callback_t callbackSync,
                                       log_callback_t callbackLog,
                                       const char* dbPath,
                                       const char* normalizerConfigPath,
                                       const char* normalizerType,
                                       const bool scanOnStart,
                                       const bool hardware,
                                       const bool os,
                                       const bool network,
                                       const bool packages,
                                       const bool ports,
                                       const bool portsAll,
                                       const bool processes,
                                       const bool hotfixes);

typedef void(*inventory_stop_func)();

typedef int (*inventory_sync_message_func)(const char* data);

typedef void (*rsync_initialize_full_log_func)(full_log_fnc_t log_function);

#endif //_INVENTORY_H
