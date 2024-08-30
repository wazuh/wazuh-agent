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

void Inventory::start() {

    if (!m_enabled) {
        log(LOG_INFO, "Module disabled. Exiting...");
        pthread_exit(NULL);
    }

    log(LOG_INFO, "Starting inventory.");

    showConfig();

    DBSync::initialize(logError);

    try
    {
        Inventory::instance().init(std::make_shared<SysInfo>(),
                                    [this](const std::string& diff) { this->sendDeltaEvent(diff); },
                                    INVENTORY_DB_DISK_PATH,
                                    INVENTORY_NORM_CONFIG_DISK_PATH,
                                    INVENTORY_NORM_TYPE);
    }
    catch (const std::exception& ex)
    {
        logError(ex.what());
    }

    log(LOG_INFO, "Module finished.");

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
    log(LOG_INFO, "Module stopped.");
    Inventory::instance().destroy();
}

string Inventory::command(const string & query) {
    log(LOG_INFO, "Query: " + query);
    return "OK";
}

string Inventory::name() const {
    return "inventory";
}

void Inventory::setMessageQueue(const std::shared_ptr<IMultiTypeQueue> queue) {
    m_messageQueue = queue;
}

void Inventory::sendDeltaEvent(const string& data) {

    const auto jsonData = nlohmann::json::parse(data);
    const Message message{ MessageType::STATELESS, jsonData, name() };

    if(!m_messageQueue->push(message)) {
        log(LOG_WARNING, "Delta event can't be pushed into the message queue: " + data);
    }
    else {
        log(LOG_DEBUG_VERBOSE, "Delta sent: " + data);
    }
}

void Inventory::showConfig()
{
    cJSON * configJson = dump();
    if (configJson) {
        char * configString = cJSON_PrintUnformatted(configJson);
        if (configString) {
            log(LOG_DEBUG, configString);
            cJSON_free(configString);
        }
        cJSON_Delete(configJson);
    }
}

cJSON * Inventory::dump() {

    cJSON *rootJson = cJSON_CreateObject();
    cJSON *invJson = cJSON_CreateObject();

    // System provider values
    if (m_enabled) cJSON_AddStringToObject(invJson,"disabled","no"); else cJSON_AddStringToObject(invJson,"disabled","yes");
    if (m_scanOnStart) cJSON_AddStringToObject(invJson,"scan-on-start","yes"); else cJSON_AddStringToObject(invJson,"scan-on-start","no");
    cJSON_AddNumberToObject(invJson,"interval",m_intervalValue.count());
    if (m_network) cJSON_AddStringToObject(invJson,"network","yes"); else cJSON_AddStringToObject(invJson,"network","no");
    if (m_os) cJSON_AddStringToObject(invJson,"os","yes"); else cJSON_AddStringToObject(invJson,"os","no");
    if (m_hardware) cJSON_AddStringToObject(invJson,"hardware","yes"); else cJSON_AddStringToObject(invJson,"hardware","no");
    if (m_packages) cJSON_AddStringToObject(invJson,"packages","yes"); else cJSON_AddStringToObject(invJson,"packages","no");
    if (m_ports) cJSON_AddStringToObject(invJson,"ports","yes"); else cJSON_AddStringToObject(invJson,"ports","no");
    if (m_portsAll) cJSON_AddStringToObject(invJson,"ports_all","yes"); else cJSON_AddStringToObject(invJson,"ports_all","no");
    if (m_processes) cJSON_AddStringToObject(invJson,"processes","yes"); else cJSON_AddStringToObject(invJson,"processes","no");
#ifdef WIN32
    if (m_hotfixes) cJSON_AddStringToObject(invJson,"hotfixes","yes"); else cJSON_AddStringToObject(invJson,"hotfixes","no");
#endif

    cJSON_AddItemToObject(rootJson,"inventory",invJson);

    return rootJson;
}

void Inventory::log(const modules_log_level_t level, const std::string& log)
{
    taggedLogFunction(level, log.c_str(), INV_LOGTAG);
}

void Inventory::logError(const std::string& log)
{
    taggedLogFunction(LOG_ERROR, log.c_str(), INV_LOGTAG);
}
