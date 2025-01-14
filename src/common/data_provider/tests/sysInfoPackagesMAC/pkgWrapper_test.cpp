/*
 * Wazuh SysInfo
 * Copyright (C) 2015, Wazuh Inc.
 * July 20, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "pkgWrapper_test.h"
#include "packages/packageMac.h"
#include "packages/pkgWrapper.h"
#include <unistd.h>
#include <iostream>

void PKGWrapperTest::SetUp() {};

void PKGWrapperTest::TearDown() {};

using ::testing::_;
using ::testing::Return;

TEST_F(PKGWrapperTest, LongVersion)
{
    std::string inputPath;
    inputPath += getwd(NULL);
    inputPath += "/input_files";
    std::string package { "PKGWrapperTest_LongVersion.app" };

    struct PackageContext ctx
    {
        inputPath, package, ""
    };
    std::shared_ptr<PKGWrapper> wrapper;
    EXPECT_NO_THROW(wrapper = std::make_shared<PKGWrapper>(ctx));
    nlohmann::json packageJson;
    wrapper->name(packageJson);
    EXPECT_EQ(packageJson["name"], "Opera");
    wrapper->version(packageJson);
    EXPECT_EQ(packageJson["version"], "100.0.4815.54");
    wrapper->groups(packageJson);
    EXPECT_EQ(packageJson["groups"], "public.app-category.productivity");
    wrapper->description(packageJson);
    EXPECT_EQ(packageJson["description"], "com.operasoftware.Opera");
    wrapper->architecture(packageJson);
    EXPECT_EQ(packageJson["architecture"], EMPTY_VALUE);
    wrapper->format(packageJson);
    EXPECT_EQ(packageJson["format"], "pkg");
    wrapper->osPatch(packageJson);
    EXPECT_EQ(packageJson["os_patch"], EMPTY_VALUE);
    wrapper->source(packageJson);
    EXPECT_EQ(packageJson["source"], "utilities");
    wrapper->location(packageJson);
    EXPECT_EQ(packageJson["location"], inputPath + "/" + package + "/" + APP_INFO_PATH);
    wrapper->vendor(packageJson);
    EXPECT_EQ(packageJson["vendor"], "operasoftware");
    wrapper->priority(packageJson);
    EXPECT_EQ(packageJson["priority"], EMPTY_VALUE);
    wrapper->size(packageJson);
    EXPECT_EQ(packageJson["size"], EMPTY_VALUE);
    wrapper->install_time(packageJson);
    EXPECT_EQ(packageJson["install_time"], EMPTY_VALUE);
    wrapper->multiarch(packageJson);
    EXPECT_EQ(packageJson["multiarch"], EMPTY_VALUE);
}

TEST_F(PKGWrapperTest, ShortVersion)
{
    std::string inputPath;
    inputPath += getwd(NULL);
    inputPath += "/input_files";
    std::string package { "PKGWrapperTest_ShortVersion.app" };

    struct PackageContext ctx
    {
        inputPath, package, ""
    };
    std::shared_ptr<PKGWrapper> wrapper;
    EXPECT_NO_THROW(wrapper = std::make_shared<PKGWrapper>(ctx));
    nlohmann::json packageJson;
    wrapper->name(packageJson);
    EXPECT_EQ(packageJson["name"], "Opera");
    wrapper->version(packageJson);
    EXPECT_EQ(packageJson["version"], "100.0");
    wrapper->groups(packageJson);
    EXPECT_EQ(packageJson["groups"], "public.app-category.productivity");
    wrapper->description(packageJson);
    EXPECT_EQ(packageJson["description"], "com.operasoftware.Opera");
    wrapper->architecture(packageJson);
    EXPECT_EQ(packageJson["architecture"], EMPTY_VALUE);
    wrapper->format(packageJson);
    EXPECT_EQ(packageJson["format"], "pkg");
    wrapper->osPatch(packageJson);
    EXPECT_EQ(packageJson["os_patch"], EMPTY_VALUE);
    wrapper->source(packageJson);
    EXPECT_EQ(packageJson["source"], "utilities");
    wrapper->location(packageJson);
    EXPECT_EQ(packageJson["location"], inputPath + "/" + package + "/" + APP_INFO_PATH);
    wrapper->vendor(packageJson);
    EXPECT_EQ(packageJson["vendor"], "operasoftware");
    wrapper->priority(packageJson);
    EXPECT_EQ(packageJson["priority"], EMPTY_VALUE);
    wrapper->size(packageJson);
    EXPECT_EQ(packageJson["size"], EMPTY_VALUE);
    wrapper->install_time(packageJson);
    EXPECT_EQ(packageJson["install_time"], EMPTY_VALUE);
    wrapper->multiarch(packageJson);
    EXPECT_EQ(packageJson["multiarch"], EMPTY_VALUE);
}

TEST_F(PKGWrapperTest, NoName)
{
    std::string inputPath;
    inputPath += getwd(NULL);
    inputPath += "/input_files";
    std::string package { "PKGWrapperTest_NoName.app" };

    struct PackageContext ctx
    {
        inputPath, package, ""
    };
    std::shared_ptr<PKGWrapper> wrapper;
    EXPECT_NO_THROW(wrapper = std::make_shared<PKGWrapper>(ctx));
    nlohmann::json packageJson;
    wrapper->name(packageJson);
    EXPECT_EQ(packageJson["name"], "");
    wrapper->version(packageJson);
    EXPECT_EQ(packageJson["version"], "100.0.4815.54");
    wrapper->groups(packageJson);
    EXPECT_EQ(packageJson["groups"], "public.app-category.productivity");
    wrapper->description(packageJson);
    EXPECT_EQ(packageJson["description"], "com.operasoftware.Opera");
    wrapper->architecture(packageJson);
    EXPECT_EQ(packageJson["architecture"], EMPTY_VALUE);
    wrapper->format(packageJson);
    EXPECT_EQ(packageJson["format"], "pkg");
    wrapper->osPatch(packageJson);
    EXPECT_EQ(packageJson["os_patch"], EMPTY_VALUE);
    wrapper->source(packageJson);
    EXPECT_EQ(packageJson["source"], "utilities");
    wrapper->location(packageJson);
    EXPECT_EQ(packageJson["location"], inputPath + "/" + package + "/" + APP_INFO_PATH);
    wrapper->vendor(packageJson);
    EXPECT_EQ(packageJson["vendor"], "operasoftware");
    wrapper->priority(packageJson);
    EXPECT_EQ(packageJson["priority"], EMPTY_VALUE);
    wrapper->size(packageJson);
    EXPECT_EQ(packageJson["size"], EMPTY_VALUE);
    wrapper->install_time(packageJson);
    EXPECT_EQ(packageJson["install_time"], EMPTY_VALUE);
    wrapper->multiarch(packageJson);
    EXPECT_EQ(packageJson["multiarch"], EMPTY_VALUE);
}

TEST_F(PKGWrapperTest, NoVersion)
{
    std::string inputPath;
    inputPath += getwd(NULL);
    inputPath += "/input_files";
    std::string package { "PKGWrapperTest_NoVersion.app" };

    struct PackageContext ctx
    {
        inputPath, package, ""
    };
    std::shared_ptr<PKGWrapper> wrapper;
    EXPECT_NO_THROW(wrapper = std::make_shared<PKGWrapper>(ctx));
    nlohmann::json packageJson;
    wrapper->name(packageJson);
    EXPECT_EQ(packageJson["name"], "Opera");
    wrapper->version(packageJson);
    EXPECT_EQ(packageJson["version"], EMPTY_VALUE);
    wrapper->groups(packageJson);
    EXPECT_EQ(packageJson["groups"], "public.app-category.productivity");
    wrapper->description(packageJson);
    EXPECT_EQ(packageJson["description"], "com.operasoftware.Opera");
    wrapper->architecture(packageJson);
    EXPECT_EQ(packageJson["architecture"], EMPTY_VALUE);
    wrapper->format(packageJson);
    EXPECT_EQ(packageJson["format"], "pkg");
    wrapper->osPatch(packageJson);
    EXPECT_EQ(packageJson["os_patch"], EMPTY_VALUE);
    wrapper->source(packageJson);
    EXPECT_EQ(packageJson["source"], "utilities");
    wrapper->location(packageJson);
    EXPECT_EQ(packageJson["location"], inputPath + "/" + package + "/" + APP_INFO_PATH);
    wrapper->vendor(packageJson);
    EXPECT_EQ(packageJson["vendor"], "operasoftware");
    wrapper->priority(packageJson);
    EXPECT_EQ(packageJson["priority"], EMPTY_VALUE);
    wrapper->size(packageJson);
    EXPECT_EQ(packageJson["size"], EMPTY_VALUE);
    wrapper->install_time(packageJson);
    EXPECT_EQ(packageJson["install_time"], EMPTY_VALUE);
    wrapper->multiarch(packageJson);
    EXPECT_EQ(packageJson["multiarch"], EMPTY_VALUE);
}

TEST_F(PKGWrapperTest, NoGroups)
{
    std::string inputPath;
    inputPath += getwd(NULL);
    inputPath += "/input_files";
    std::string package { "PKGWrapperTest_NoGroups.app" };

    struct PackageContext ctx
    {
        inputPath, package, ""
    };
    std::shared_ptr<PKGWrapper> wrapper;
    EXPECT_NO_THROW(wrapper = std::make_shared<PKGWrapper>(ctx));
    nlohmann::json packageJson;
    wrapper->name(packageJson);
    EXPECT_EQ(packageJson["name"], "Opera");
    wrapper->version(packageJson);
    EXPECT_EQ(packageJson["version"], "100.0.4815.54");
    wrapper->groups(packageJson);
    EXPECT_EQ(packageJson["groups"], EMPTY_VALUE);
    wrapper->description(packageJson);
    EXPECT_EQ(packageJson["description"], "com.operasoftware.Opera");
    wrapper->architecture(packageJson);
    EXPECT_EQ(packageJson["architecture"], EMPTY_VALUE);
    wrapper->format(packageJson);
    EXPECT_EQ(packageJson["format"], "pkg");
    wrapper->osPatch(packageJson);
    EXPECT_EQ(packageJson["os_patch"], EMPTY_VALUE);
    wrapper->source(packageJson);
    EXPECT_EQ(packageJson["source"], "utilities");
    wrapper->location(packageJson);
    EXPECT_EQ(packageJson["location"], inputPath + "/" + package + "/" + APP_INFO_PATH);
    wrapper->vendor(packageJson);
    EXPECT_EQ(packageJson["vendor"], "operasoftware");
    wrapper->priority(packageJson);
    EXPECT_EQ(packageJson["priority"], EMPTY_VALUE);
    wrapper->size(packageJson);
    EXPECT_EQ(packageJson["size"], EMPTY_VALUE);
    wrapper->install_time(packageJson);
    EXPECT_EQ(packageJson["install_time"], EMPTY_VALUE);
    wrapper->multiarch(packageJson);
    EXPECT_EQ(packageJson["multiarch"], EMPTY_VALUE);
}

TEST_F(PKGWrapperTest, NoDescription)
{
    std::string inputPath;
    inputPath += getwd(NULL);
    inputPath += "/input_files";
    std::string package { "PKGWrapperTest_NoDescription.app" };

    struct PackageContext ctx
    {
        inputPath, package, ""
    };
    std::shared_ptr<PKGWrapper> wrapper;
    EXPECT_NO_THROW(wrapper = std::make_shared<PKGWrapper>(ctx));
    nlohmann::json packageJson;
    wrapper->name(packageJson);
    EXPECT_EQ(packageJson["name"], "Opera");
    wrapper->version(packageJson);
    EXPECT_EQ(packageJson["version"], "100.0.4815.54");
    wrapper->groups(packageJson);
    EXPECT_EQ(packageJson["groups"], "public.app-category.productivity");
    wrapper->description(packageJson);
    EXPECT_EQ(packageJson["description"], EMPTY_VALUE);
    wrapper->architecture(packageJson);
    EXPECT_EQ(packageJson["architecture"], EMPTY_VALUE);
    wrapper->format(packageJson);
    EXPECT_EQ(packageJson["format"], "pkg");
    wrapper->osPatch(packageJson);
    EXPECT_EQ(packageJson["os_patch"], EMPTY_VALUE);
    wrapper->source(packageJson);
    EXPECT_EQ(packageJson["source"], "utilities");
    wrapper->location(packageJson);
    EXPECT_EQ(packageJson["location"], inputPath + "/" + package + "/" + APP_INFO_PATH);
    wrapper->vendor(packageJson);
    EXPECT_EQ(packageJson["vendor"], EMPTY_VALUE);
    wrapper->priority(packageJson);
    EXPECT_EQ(packageJson["priority"], EMPTY_VALUE);
    wrapper->size(packageJson);
    EXPECT_EQ(packageJson["size"], EMPTY_VALUE);
    wrapper->install_time(packageJson);
    EXPECT_EQ(packageJson["install_time"], EMPTY_VALUE);
    wrapper->multiarch(packageJson);
    EXPECT_EQ(packageJson["multiarch"], EMPTY_VALUE);
}

TEST_F(PKGWrapperTest, NoVendor)
{
    std::string inputPath;
    inputPath += getwd(NULL);
    inputPath += "/input_files";
    std::string package { "PKGWrapperTest_NoVendor.app" };

    struct PackageContext ctx
    {
        inputPath, package, ""
    };
    std::shared_ptr<PKGWrapper> wrapper;
    EXPECT_NO_THROW(wrapper = std::make_shared<PKGWrapper>(ctx));
    nlohmann::json packageJson;
    wrapper->name(packageJson);
    EXPECT_EQ(packageJson["name"], "Opera");
    wrapper->version(packageJson);
    EXPECT_EQ(packageJson["version"], "100.0.4815.54");
    wrapper->groups(packageJson);
    EXPECT_EQ(packageJson["groups"], "public.app-category.productivity");
    wrapper->description(packageJson);
    EXPECT_EQ(packageJson["description"], "description_text");
    wrapper->architecture(packageJson);
    EXPECT_EQ(packageJson["architecture"], EMPTY_VALUE);
    wrapper->format(packageJson);
    EXPECT_EQ(packageJson["format"], "pkg");
    wrapper->osPatch(packageJson);
    EXPECT_EQ(packageJson["os_patch"], EMPTY_VALUE);
    wrapper->source(packageJson);
    EXPECT_EQ(packageJson["source"], "utilities");
    wrapper->location(packageJson);
    EXPECT_EQ(packageJson["location"], inputPath + "/" + package + "/" + APP_INFO_PATH);
    wrapper->vendor(packageJson);
    EXPECT_EQ(packageJson["vendor"], EMPTY_VALUE);
    wrapper->priority(packageJson);
    EXPECT_EQ(packageJson["priority"], EMPTY_VALUE);
    wrapper->size(packageJson);
    EXPECT_EQ(packageJson["size"], EMPTY_VALUE);
    wrapper->install_time(packageJson);
    EXPECT_EQ(packageJson["install_time"], EMPTY_VALUE);
    wrapper->multiarch(packageJson);
    EXPECT_EQ(packageJson["multiarch"], EMPTY_VALUE);
}

TEST_F(PKGWrapperTest, pkgVersionXML)
{
    std::string inputPath;
    inputPath += getwd(NULL);
    inputPath += "/input_files";
    std::string package { "com.wazuh.pkg.wazuh-agent.plist" };

    struct PackageContext ctx
    {
        inputPath, package, ""
    };
    std::shared_ptr<PKGWrapper> wrapper;
    EXPECT_NO_THROW(wrapper = std::make_shared<PKGWrapper>(ctx));
    nlohmann::json packageJson;
    wrapper->name(packageJson);
    EXPECT_EQ(packageJson["name"], "wazuh-agent");
    wrapper->version(packageJson);
    EXPECT_EQ(packageJson["version"], "4.10.1");
    wrapper->groups(packageJson);
    EXPECT_EQ(packageJson["groups"], EMPTY_VALUE);
    wrapper->description(packageJson);
    EXPECT_EQ(packageJson["description"], "com.wazuh.pkg.wazuh-agent");
    wrapper->architecture(packageJson);
    EXPECT_EQ(packageJson["architecture"], EMPTY_VALUE);
    wrapper->format(packageJson);
    EXPECT_EQ(packageJson["format"], "pkg");
    wrapper->osPatch(packageJson);
    EXPECT_EQ(packageJson["os_patch"], EMPTY_VALUE);
    wrapper->source(packageJson);
    EXPECT_EQ(packageJson["source"], "receipts");
    wrapper->location(packageJson);
    EXPECT_EQ(packageJson["location"], inputPath + "/" + package);
    wrapper->vendor(packageJson);
    EXPECT_EQ(packageJson["vendor"], "wazuh");
    wrapper->priority(packageJson);
    EXPECT_EQ(packageJson["priority"], EMPTY_VALUE);
    wrapper->size(packageJson);
    EXPECT_EQ(packageJson["size"], EMPTY_VALUE);
    wrapper->install_time(packageJson);
    EXPECT_EQ(packageJson["install_time"], "2024-11-07T08:58:38Z");
    wrapper->multiarch(packageJson);
    EXPECT_EQ(packageJson["multiarch"], EMPTY_VALUE);
}

TEST_F(PKGWrapperTest, pkgVersionBin)
{
    std::string inputPath;
    inputPath += getwd(NULL);
    inputPath += "/input_files";
    std::string package { "us.zoom.pkg.videomeeting.plist" };

    struct PackageContext ctx
    {
        inputPath, package, ""
    };
    std::shared_ptr<PKGWrapper> wrapper;
    EXPECT_NO_THROW(wrapper = std::make_shared<PKGWrapper>(ctx));
    nlohmann::json packageJson;
    wrapper->name(packageJson);
    EXPECT_EQ(packageJson["name"], "videomeeting");
    wrapper->version(packageJson);
    EXPECT_EQ(packageJson["version"], "6.2.6.41824");
    wrapper->groups(packageJson);
    EXPECT_EQ(packageJson["groups"], EMPTY_VALUE);
    wrapper->description(packageJson);
    EXPECT_EQ(packageJson["description"], "us.zoom.pkg.videomeeting");
    wrapper->architecture(packageJson);
    EXPECT_EQ(packageJson["architecture"], EMPTY_VALUE);
    wrapper->format(packageJson);
    EXPECT_EQ(packageJson["format"], "pkg");
    wrapper->osPatch(packageJson);
    EXPECT_EQ(packageJson["os_patch"], EMPTY_VALUE);
    wrapper->source(packageJson);
    EXPECT_EQ(packageJson["source"], "receipts");
    wrapper->location(packageJson);
    EXPECT_EQ(packageJson["location"], inputPath + "/" + package);
    wrapper->vendor(packageJson);
    EXPECT_EQ(packageJson["vendor"], "zoom");
    wrapper->priority(packageJson);
    EXPECT_EQ(packageJson["priority"], EMPTY_VALUE);
    wrapper->size(packageJson);
    EXPECT_EQ(packageJson["size"], EMPTY_VALUE);
    wrapper->install_time(packageJson);
    EXPECT_EQ(packageJson["install_time"], "2024-11-08T11:44:04Z");
    wrapper->multiarch(packageJson);
    EXPECT_EQ(packageJson["multiarch"], EMPTY_VALUE);
}

TEST_F(PKGWrapperTest, pkgVersionLong)
{
    std::string inputPath;
    inputPath += getwd(NULL);
    inputPath += "/input_files";
    std::string package { "org.R-project.x86_64.R.GUI.pkg.plist" };

    struct PackageContext ctx
    {
        inputPath, package, ""
    };
    std::shared_ptr<PKGWrapper> wrapper;
    EXPECT_NO_THROW(wrapper = std::make_shared<PKGWrapper>(ctx));
    nlohmann::json packageJson;
    wrapper->name(packageJson);
    EXPECT_EQ(packageJson["name"], "R.GUI");
    wrapper->version(packageJson);
    EXPECT_EQ(packageJson["version"], "1.81");
    wrapper->groups(packageJson);
    EXPECT_EQ(packageJson["groups"], EMPTY_VALUE);
    wrapper->description(packageJson);
    EXPECT_EQ(packageJson["description"], "org.R-project.x86_64.R.GUI.pkg");
    wrapper->architecture(packageJson);
    EXPECT_EQ(packageJson["architecture"], EMPTY_VALUE);
    wrapper->format(packageJson);
    EXPECT_EQ(packageJson["format"], "pkg");
    wrapper->osPatch(packageJson);
    EXPECT_EQ(packageJson["os_patch"], EMPTY_VALUE);
    wrapper->source(packageJson);
    EXPECT_EQ(packageJson["source"], "receipts");
    wrapper->location(packageJson);
    EXPECT_EQ(packageJson["location"], inputPath + "/" + package);
    wrapper->vendor(packageJson);
    EXPECT_EQ(packageJson["vendor"], "R-project");
    wrapper->priority(packageJson);
    EXPECT_EQ(packageJson["priority"], EMPTY_VALUE);
    wrapper->size(packageJson);
    EXPECT_EQ(packageJson["size"], EMPTY_VALUE);
    wrapper->install_time(packageJson);
    EXPECT_EQ(packageJson["install_time"], "2024-11-13T10:59:10Z");
    wrapper->multiarch(packageJson);
    EXPECT_EQ(packageJson["multiarch"], EMPTY_VALUE);
}
