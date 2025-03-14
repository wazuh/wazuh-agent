/*
 * Wazuh SysInfo
 * Copyright (C) 2015, Wazuh Inc.
 * November 23, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */
#include "osinfo/sysOsParsers.h"
#include "sharedDefs.h"
#include "sysInfo.hpp"
#include "timeHelper.hpp"
#include <fstream>
#include <sys/utsname.h>

static void getOsInfoFromUname(nlohmann::json& info)
{
    info["os_name"] = "Unix";
    info["os_platform"] = "Unix";
    info["os_version"] = UNKNOWN_VALUE;
}

static std::string getSerialNumber()
{
    return EMPTY_VALUE;
}

static std::string getCpuName()
{
    return EMPTY_VALUE;
}

static int getCpuMHz()
{
    return 0;
}

static int getCpuCores()
{
    return 0;
}

static void getMemory(nlohmann::json& /*info*/) {}

nlohmann::json SysInfo::getHardware() const
{
    nlohmann::json hardware;
    hardware["board_serial"] = getSerialNumber();
    hardware["cpu_name"] = getCpuName();
    hardware["cpu_cores"] = getCpuCores();
    hardware["cpu_mhz"] = getCpuMHz();
    getMemory(hardware);
    return hardware;
}

nlohmann::json SysInfo::getPackages() const
{
    return nlohmann::json {};
}

nlohmann::json SysInfo::getOsInfo() const
{
    nlohmann::json ret;

    struct utsname uts
    {
    };

    getOsInfoFromUname(ret);

    if (uname(&uts) >= 0)
    {
        ret["sysname"] = uts.sysname;
        ret["hostname"] = uts.nodename;
        ret["version"] = uts.version;
        ret["architecture"] = uts.machine;
        ret["release"] = uts.release;
    }

    return ret;
}

nlohmann::json SysInfo::getProcessesInfo() const
{
    return nlohmann::json();
}

nlohmann::json SysInfo::getNetworks() const
{
    return nlohmann::json();
}

nlohmann::json SysInfo::getPorts() const
{
    return nlohmann::json();
}

void SysInfo::getProcessesInfo(std::function<void(nlohmann::json&)> /*callback*/) const
{
    // TODO
}

void SysInfo::getPackages(std::function<void(nlohmann::json&)> /*callback*/) const
{
    // TODO
}

nlohmann::json SysInfo::getHotfixes() const
{
    // Currently not supported for this OS.
    return nlohmann::json();
}
