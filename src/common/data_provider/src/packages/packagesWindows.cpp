/*
 * Wazuh SYSINFO
 * Copyright (C) 2015, Wazuh Inc.
 * January 24, 2022.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "packagesWindows.h"
#include "appxWindowsWrapper.h"
#include "sharedDefs.h"

std::shared_ptr<IPackage> FactoryWindowsPackage::create(const HKEY key,
                                                        const std::string& userId,
                                                        const std::string& nameApp,
                                                        const std::set<std::string>& cacheRegistry)
{
    return std::make_shared<WindowsPackageImpl>(
        std::make_shared<AppxWindowsWrapper>(key, userId, nameApp, cacheRegistry));
}

WindowsPackageImpl::WindowsPackageImpl(const std::shared_ptr<IPackageWrapper>& packageWrapper)
    : m_packageWrapper(packageWrapper)
{
}

void WindowsPackageImpl::buildPackageData(nlohmann::json& package)
{
    m_packageWrapper->name(package);
    m_packageWrapper->version(package);
    m_packageWrapper->vendor(package);
    m_packageWrapper->install_time(package);
    m_packageWrapper->location(package);
    m_packageWrapper->architecture(package);
    m_packageWrapper->groups(package);
    m_packageWrapper->description(package);
    m_packageWrapper->size(package);
    m_packageWrapper->priority(package);
    m_packageWrapper->source(package);
    m_packageWrapper->format(package);
}
