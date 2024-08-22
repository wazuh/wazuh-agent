/*
 * Wazuh InventoryImp
 * Copyright (C) 2015, Wazuh Inc.
 * November 9, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */
#include <cstdio>
#include <gtest/gtest.h>
#include "inventoryImp_test.hpp"
#include "inventory.hpp"

constexpr auto INVENTORY_DB_PATH {"TEMP.db"};

void InventoryImpTest::SetUp() {};

void InventoryImpTest::TearDown()
{
    std::remove(INVENTORY_DB_PATH);
};

using ::testing::_;
using ::testing::Return;

class SysInfoWrapper: public ISysInfo
{
    public:
        SysInfoWrapper() = default;
        ~SysInfoWrapper() = default;
        MOCK_METHOD(nlohmann::json, hardware, (), (override));
        MOCK_METHOD(nlohmann::json, packages, (), (override));
        MOCK_METHOD(void, packages, (std::function<void(nlohmann::json&)>), (override));
        MOCK_METHOD(nlohmann::json, os, (), (override));
        MOCK_METHOD(nlohmann::json, networks, (), (override));
        MOCK_METHOD(nlohmann::json, processes, (), (override));
        MOCK_METHOD(void, processes, (std::function<void(nlohmann::json&)>), (override));
        MOCK_METHOD(nlohmann::json, ports, (), (override));
        MOCK_METHOD(nlohmann::json, hotfixes, (), (override));
};

class CallbackMock
{
    public:
        CallbackMock() = default;
        ~CallbackMock() = default;
        MOCK_METHOD(void, callbackMock, (const std::string&), ());
};

void reportFunction(const std::string& /*payload*/)
{
    // std::cout << payload << std::endl;
}

void logFunction(const modules_log_level_t /*level*/, const std::string& /*log*/)
{
    // static const std::map<modules_log_level_t, std::string> s_logStringMap
    // {
    //     {LOG_ERROR, "ERROR"},
    //     {LOG_INFO, "INFO"},
    //     {LOG_DEBUG, "DEBUG"},
    //     {LOG_DEBUG_VERBOSE, "DEBUG2"}
    // };
    // std::cout << s_logStringMap.at(level) << ": " << log << std::endl;
}

TEST_F(InventoryImpTest, defaultCtor)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};
    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})")));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})")));
    EXPECT_CALL(*spInfoWrapper, networks()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"iface":[{"address":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "mac":"d4:5d:64:51:07:5d", "gateway":"192.168.0.1|600","broadcast":"127.255.255.255", "name":"ens1", "mtu":1500, "name":"enp4s0", "adapter":" ", "type":"ethernet", "state":"up", "dhcp":"disabled","iface":"Loopback Pseudo-Interface 1","metric":"75","netmask":"255.0.0.0","proto":"IPv4","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0, "IPv4":[{"address":"192.168.153.1","broadcast":"192.168.153.255","dhcp":"unknown","metric":" ","netmask":"255.255.255.0"}], "IPv6":[{"address":"fe80::250:56ff:fec0:8","dhcp":"unknown","metric":" ","netmask":"ffff:ffff:ffff:ffff::"}]}]})")));
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                   R"([{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0}])")));
    EXPECT_CALL(*spInfoWrapper, packages(_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));

    EXPECT_CALL(*spInfoWrapper, hotfixes()).WillRepeatedly(Return(R"([{"hotfix":"KB12345678"}])"_json));

    EXPECT_CALL(*spInfoWrapper, processes(_))
    .Times(testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","scan_time":"2020/12/28 21:49:50", "nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0})"_json));

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);

            if (delta.at("type").get_ref<const std::string&>().compare("dbsync_osinfo") == 0)
            {
                delta["data"].erase("checksum");
            }

            delta["data"].erase("scan_time");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"board_serial":"Intel Corporation","checksum":"af7b22eef8f5e06c04af4db49c9f8d1d28963918","cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","ram_free":2257872,"ram_total":4972208,"ram_usage":54},"operation":"INSERTED","type":"dbsync_hwinfo"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"architecture":"x86_64","hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"},"operation":"INSERTED","type":"dbsync_osinfo"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"adapter":" ","checksum":"165f7160ecd2838479ee4c43c1012b723736d90a","item_id":"25eef9a0a422a9b644fb6b73650453148bc6151c","mac":"d4:5d:64:51:07:5d","mtu":1500,"name":"enp4s0","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"state":"up","tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0,"type":"ethernet"},"operation":"INSERTED","type":"dbsync_network_iface"})"
    };
    const auto expectedResult4
    {
        R"({"data":{"checksum":"ff63981c231f110a0877ac6acd8862ac09877b5d","dhcp":"unknown","gateway":"192.168.0.1|600","iface":"enp4s0","item_id":"d633b040008ea38303d778431ee2fd0b4ee5a37a","metric":" ","type":"ipv4"},"operation":"INSERTED","type":"dbsync_network_protocol"})"
    };
    const auto expectedResult5
    {
        R"({"data":{"address":"192.168.153.1","broadcast":"192.168.153.255","checksum":"72dfd66759bd8062cdc17607d760a48c906189b3","iface":"enp4s0","item_id":"3d48ddc47fac84c62a19746af66fbfcf78547de9","netmask":"255.255.255.0","proto":0},"operation":"INSERTED","type":"dbsync_network_address"})"
    };
    const auto expectedResult6
    {
        R"({"data":{"address":"fe80::250:56ff:fec0:8","checksum":"f606d1a1c551874d8fab33e4e5cfaa0370673ec8","iface":"enp4s0","item_id":"65973316a5dc8615a6d20b2d6c4ce52ecd074496","netmask":"ffff:ffff:ffff:ffff::","proto":1},"operation":"INSERTED","type":"dbsync_network_address"})"
    };
    const auto expectedResult7
    {
        R"({"data":{"checksum":"f25348b1ce5310f36c1ed859d13138fbb4e6bacb","inode":0,"item_id":"cbf2ac25a6775175f912ebf2abc72f6f51ab48ba","local_ip":"127.0.0.1","local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0},"operation":"INSERTED","type":"dbsync_ports"})"
    };
    const auto expectedResult8
    {
        R"({"data":{"checksum":"039934723aa69928b52e470c8d27365b0924b615","egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0},"operation":"INSERTED","type":"dbsync_processes"})"
    };
    const auto expectedResult9
    {
        R"({"data":{"architecture":"amd64","checksum":"c084f78ed87ed19974b1fd90bbf727c2d1416f7d","format":"deb","group":"x11","item_id":"4846c220a185b0fc251a07843efbfbb0d90ac4a5","location":" ","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14"},"operation":"INSERTED","type":"dbsync_packages"})"
    };

    const auto expectedResult10
    {
        R"({"data":{"checksum":"56162cd7bb632b4728ec868e8e271b01222ff131","hotfix":"KB12345678"},"operation":"INSERTED","type":"dbsync_hotfixes"})"
    };

    const auto expectedResult11
    {
        R"({"data":{"checksum":"ea17673e7422c0ab04c4f1f111a5828be8cd366a","dhcp":"unknown","gateway":"192.168.0.1|600","iface":"enp4s0","item_id":"9dff246584835755137820c975f034d089e90b6f","metric":" ","type":"ipv6"},"operation":"INSERTED","type":"dbsync_network_protocol"})"
    };


    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult4)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult5)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult6)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult7)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult8)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult9)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult10)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult11)).Times(1);

    Configuration config;
    Inventory::instance().setup(config);
    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::instance().init(spInfoWrapper,
                                        callbackDataDelta,
                                        INVENTORY_DB_PATH,
                                        "",
                                        "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds(1));
    Inventory::instance().stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, intervalSeconds)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};
    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","ram_free":2257872,"ram_total":4972208,"ram_usage":54})")));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})")));
    EXPECT_CALL(*spInfoWrapper, networks()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"iface":[{"address":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "mac":"d4:5d:64:51:07:5d", "gateway":"192.168.0.1|600","broadcast":"127.255.255.255", "name":"ens1", "mtu":1500, "name":"enp4s0", "adapter":" ", "type":"ethernet", "state":"up", "dhcp":"disabled","iface":"Loopback Pseudo-Interface 1","metric":"75","netmask":"255.0.0.0","proto":"IPv4","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0, "IPv4":[{"address":"192.168.153.1","broadcast":"192.168.153.255","dhcp":"unknown","metric":" ","netmask":"255.255.255.0"}], "IPv6":[{"address":"fe80::250:56ff:fec0:8","dhcp":"unknown","metric":" ","netmask":"ffff:ffff:ffff:ffff::"}]}]})")));
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                   R"([{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0}])")));
    EXPECT_CALL(*spInfoWrapper, hotfixes()).WillRepeatedly(Return(R"([{"hotfix":"KB12345678"},{"hotfix":"KB87654321"}])"_json));
    EXPECT_CALL(*spInfoWrapper, processes(_))
    .Times(::testing::AtLeast(2))
    .WillRepeatedly(testing::InvokeArgument<0>(nlohmann::json::parse(
                                                   R"({"egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","scan_time":"2020/12/28 21:49:50", "nice":0,"nlwp":1,"pgrp":0,"pid":431625,"ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0})")));

    EXPECT_CALL(*spInfoWrapper, packages(_))
    .Times(::testing::AtLeast(2))
    .WillRepeatedly(::testing::InvokeArgument<0>
                    (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));

    InventoryConfig invConfig;
    invConfig.interval = 1;
    Configuration config(invConfig);
    Inventory::instance().setup(config);

    std::thread t
    {
        [&spInfoWrapper]()
        {
            Inventory::instance().init(spInfoWrapper,
                                          reportFunction,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");

        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{10});
    Inventory::instance().stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noScanOnStart)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};
    EXPECT_CALL(*spInfoWrapper, hardware()).Times(0);
    EXPECT_CALL(*spInfoWrapper, os()).Times(0);
    EXPECT_CALL(*spInfoWrapper, packages(_)).Times(0);
    EXPECT_CALL(*spInfoWrapper, networks()).Times(0);
    EXPECT_CALL(*spInfoWrapper, processes(_)).Times(0);
    EXPECT_CALL(*spInfoWrapper, ports()).Times(0);
    EXPECT_CALL(*spInfoWrapper, hotfixes()).Times(0);

    InventoryConfig invConfig;
    invConfig.scanOnStart = false;
    Configuration config(invConfig);
    Inventory::instance().setup(config);

    std::thread t
    {
        [&spInfoWrapper]()
        {
            Inventory::instance().init(spInfoWrapper,
                                        reportFunction,
                                        INVENTORY_DB_PATH,
                                        "",
                                        "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::instance().stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noHardware)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};
    EXPECT_CALL(*spInfoWrapper, hardware()).Times(0);
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})")));
    EXPECT_CALL(*spInfoWrapper, networks()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"iface":[{"address":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "mac":"d4:5d:64:51:07:5d", "gateway":"192.168.0.1|600","broadcast":"127.255.255.255", "name":"ens1", "mtu":1500, "name":"enp4s0", "adapter":" ", "type":"ethernet", "state":"up", "dhcp":"disabled","iface":"Loopback Pseudo-Interface 1","metric":"75","netmask":"255.0.0.0","proto":"IPv4","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0, "IPv4":[{"address":"192.168.153.1","broadcast":"192.168.153.255","dhcp":"unknown","metric":" ","netmask":"255.255.255.0"}], "IPv6":[{"address":"fe80::250:56ff:fec0:8","dhcp":"unknown","metric":" ","netmask":"ffff:ffff:ffff:ffff::"}]}]})")));
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                   R"([{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0}])")));
    EXPECT_CALL(*spInfoWrapper, packages(_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));

    EXPECT_CALL(*spInfoWrapper, hotfixes()).WillRepeatedly(Return(R"([{"hotfix":"KB12345678"}])"_json));

    EXPECT_CALL(*spInfoWrapper, processes(_))
    .Times(testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","scan_time":"2020/12/28 21:49:50", "nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0})"_json));

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);

            if (delta["type"].get_ref<const std::string&>().compare("dbsync_osinfo") == 0)
            {
                delta["data"].erase("checksum");
            }

            delta["data"].erase("scan_time");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult2
    {
        R"({"data":{"architecture":"x86_64","hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"},"operation":"INSERTED","type":"dbsync_osinfo"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"adapter":" ","checksum":"165f7160ecd2838479ee4c43c1012b723736d90a","item_id":"25eef9a0a422a9b644fb6b73650453148bc6151c","mac":"d4:5d:64:51:07:5d","mtu":1500,"name":"enp4s0","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"state":"up","tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0,"type":"ethernet"},"operation":"INSERTED","type":"dbsync_network_iface"})"
    };
    const auto expectedResult4
    {
        R"({"data":{"checksum":"ff63981c231f110a0877ac6acd8862ac09877b5d","dhcp":"unknown","gateway":"192.168.0.1|600","iface":"enp4s0","item_id":"d633b040008ea38303d778431ee2fd0b4ee5a37a","metric":" ","type":"ipv4"},"operation":"INSERTED","type":"dbsync_network_protocol"})"
    };
    const auto expectedResult5
    {
        R"({"data":{"address":"192.168.153.1","broadcast":"192.168.153.255","checksum":"72dfd66759bd8062cdc17607d760a48c906189b3","iface":"enp4s0","item_id":"3d48ddc47fac84c62a19746af66fbfcf78547de9","netmask":"255.255.255.0","proto":0},"operation":"INSERTED","type":"dbsync_network_address"})"
    };
    const auto expectedResult6
    {
        R"({"data":{"address":"fe80::250:56ff:fec0:8","checksum":"f606d1a1c551874d8fab33e4e5cfaa0370673ec8","iface":"enp4s0","item_id":"65973316a5dc8615a6d20b2d6c4ce52ecd074496","netmask":"ffff:ffff:ffff:ffff::","proto":1},"operation":"INSERTED","type":"dbsync_network_address"})"
    };
    const auto expectedResult7
    {
        R"({"data":{"checksum":"f25348b1ce5310f36c1ed859d13138fbb4e6bacb","inode":0,"item_id":"cbf2ac25a6775175f912ebf2abc72f6f51ab48ba","local_ip":"127.0.0.1","local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0},"operation":"INSERTED","type":"dbsync_ports"})"
    };
    const auto expectedResult8
    {
        R"({"data":{"checksum":"039934723aa69928b52e470c8d27365b0924b615","egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0},"operation":"INSERTED","type":"dbsync_processes"})"
    };
    const auto expectedResult9
    {
        R"({"data":{"architecture":"amd64","checksum":"c084f78ed87ed19974b1fd90bbf727c2d1416f7d","format":"deb","group":"x11","item_id":"4846c220a185b0fc251a07843efbfbb0d90ac4a5","location":" ","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14"},"operation":"INSERTED","type":"dbsync_packages"})"
    };
    const auto expectedResult10
    {
        R"({"data":{"checksum":"56162cd7bb632b4728ec868e8e271b01222ff131","hotfix":"KB12345678"},"operation":"INSERTED","type":"dbsync_hotfixes"})"
    };
    const auto expectedResult11
    {
        R"({"data":{"checksum":"ea17673e7422c0ab04c4f1f111a5828be8cd366a","dhcp":"unknown","gateway":"192.168.0.1|600","iface":"enp4s0","item_id":"9dff246584835755137820c975f034d089e90b6f","metric":" ","type":"ipv6"},"operation":"INSERTED","type":"dbsync_network_protocol"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult4)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult5)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult6)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult7)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult8)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult9)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult10)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult11)).Times(1);

    InventoryConfig invConfig;
    invConfig.hardware = false;
    Configuration config(invConfig);
    Inventory::instance().setup(config);

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::instance().init(spInfoWrapper,
                                        callbackDataDelta,
                                        INVENTORY_DB_PATH,
                                        "",
                                        "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds(1));
    Inventory::instance().stop();

    if (t.joinable())
    {
        t.join();
    }

}

TEST_F(InventoryImpTest, noOs)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};
    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})")));
    EXPECT_CALL(*spInfoWrapper, networks()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"iface":[{"address":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "mac":"d4:5d:64:51:07:5d", "gateway":"192.168.0.1|600","broadcast":"127.255.255.255", "name":"ens1", "mtu":1500, "name":"enp4s0", "adapter":" ", "type":"ethernet", "state":"up", "dhcp":"disabled","iface":"Loopback Pseudo-Interface 1","metric":"75","netmask":"255.0.0.0","proto":"IPv4","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0, "IPv4":[{"address":"192.168.153.1","broadcast":"192.168.153.255","dhcp":"unknown","metric":" ","netmask":"255.255.255.0"}], "IPv6":[{"address":"fe80::250:56ff:fec0:8","dhcp":"unknown","metric":" ","netmask":"ffff:ffff:ffff:ffff::"}]}]})")));
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                   R"([{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0}])")));
    EXPECT_CALL(*spInfoWrapper, os()).Times(0);
    EXPECT_CALL(*spInfoWrapper, packages(_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));

    EXPECT_CALL(*spInfoWrapper, hotfixes()).WillRepeatedly(Return(R"([{"hotfix":"KB12345678"}])"_json));

    EXPECT_CALL(*spInfoWrapper, processes(_))
    .Times(testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","scan_time":"2020/12/28 21:49:50", "nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0})"_json));

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);

            if (delta["type"].get_ref<const std::string&>().compare("dbsync_osinfo") == 0)
            {
                delta["data"].erase("checksum");
            }

            delta["data"].erase("scan_time");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"board_serial":"Intel Corporation","checksum":"af7b22eef8f5e06c04af4db49c9f8d1d28963918","cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","ram_free":2257872,"ram_total":4972208,"ram_usage":54},"operation":"INSERTED","type":"dbsync_hwinfo"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"adapter":" ","checksum":"165f7160ecd2838479ee4c43c1012b723736d90a","item_id":"25eef9a0a422a9b644fb6b73650453148bc6151c","mac":"d4:5d:64:51:07:5d","mtu":1500,"name":"enp4s0","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"state":"up","tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0,"type":"ethernet"},"operation":"INSERTED","type":"dbsync_network_iface"})"
    };
    const auto expectedResult4
    {
        R"({"data":{"checksum":"ff63981c231f110a0877ac6acd8862ac09877b5d","dhcp":"unknown","gateway":"192.168.0.1|600","iface":"enp4s0","item_id":"d633b040008ea38303d778431ee2fd0b4ee5a37a","metric":" ","type":"ipv4"},"operation":"INSERTED","type":"dbsync_network_protocol"})"
    };
    const auto expectedResult5
    {
        R"({"data":{"address":"192.168.153.1","broadcast":"192.168.153.255","checksum":"72dfd66759bd8062cdc17607d760a48c906189b3","iface":"enp4s0","item_id":"3d48ddc47fac84c62a19746af66fbfcf78547de9","netmask":"255.255.255.0","proto":0},"operation":"INSERTED","type":"dbsync_network_address"})"
    };
    const auto expectedResult6
    {
        R"({"data":{"address":"fe80::250:56ff:fec0:8","checksum":"f606d1a1c551874d8fab33e4e5cfaa0370673ec8","iface":"enp4s0","item_id":"65973316a5dc8615a6d20b2d6c4ce52ecd074496","netmask":"ffff:ffff:ffff:ffff::","proto":1},"operation":"INSERTED","type":"dbsync_network_address"})"
    };
    const auto expectedResult7
    {
        R"({"data":{"checksum":"f25348b1ce5310f36c1ed859d13138fbb4e6bacb","inode":0,"item_id":"cbf2ac25a6775175f912ebf2abc72f6f51ab48ba","local_ip":"127.0.0.1","local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0},"operation":"INSERTED","type":"dbsync_ports"})"
    };
    const auto expectedResult8
    {
        R"({"data":{"checksum":"039934723aa69928b52e470c8d27365b0924b615","egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0},"operation":"INSERTED","type":"dbsync_processes"})"
    };
    const auto expectedResult9
    {
        R"({"data":{"architecture":"amd64","checksum":"c084f78ed87ed19974b1fd90bbf727c2d1416f7d","format":"deb","group":"x11","item_id":"4846c220a185b0fc251a07843efbfbb0d90ac4a5","location":" ","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14"},"operation":"INSERTED","type":"dbsync_packages"})"
    };
    const auto expectedResult10
    {
        R"({"data":{"checksum":"56162cd7bb632b4728ec868e8e271b01222ff131","hotfix":"KB12345678"},"operation":"INSERTED","type":"dbsync_hotfixes"})"
    };
    const auto expectedResult11
    {
        R"({"data":{"checksum":"ea17673e7422c0ab04c4f1f111a5828be8cd366a","dhcp":"unknown","gateway":"192.168.0.1|600","iface":"enp4s0","item_id":"9dff246584835755137820c975f034d089e90b6f","metric":" ","type":"ipv6"},"operation":"INSERTED","type":"dbsync_network_protocol"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult4)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult5)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult6)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult7)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult8)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult9)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult10)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult11)).Times(1);

    InventoryConfig invConfig;
    invConfig.os = false;
    Configuration config(invConfig);
    Inventory::instance().setup(config);

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::instance().init(spInfoWrapper,
                                          callbackDataDelta,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds(2));
    Inventory::instance().stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noNetwork)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};
    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})")));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})")));
    EXPECT_CALL(*spInfoWrapper, networks()).Times(0);
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                   R"([{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0}])")));
    EXPECT_CALL(*spInfoWrapper, packages(_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));

    EXPECT_CALL(*spInfoWrapper, hotfixes()).WillRepeatedly(Return(R"([{"hotfix":"KB12345678"}])"_json));

    EXPECT_CALL(*spInfoWrapper, processes(_))
    .Times(testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","scan_time":"2020/12/28 21:49:50", "nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0})"_json));

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);

            if (delta["type"].get_ref<const std::string&>().compare("dbsync_osinfo") == 0)
            {
                delta["data"].erase("checksum");
            }

            delta["data"].erase("scan_time");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"board_serial":"Intel Corporation","checksum":"af7b22eef8f5e06c04af4db49c9f8d1d28963918","cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","ram_free":2257872,"ram_total":4972208,"ram_usage":54},"operation":"INSERTED","type":"dbsync_hwinfo"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"architecture":"x86_64","hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"},"operation":"INSERTED","type":"dbsync_osinfo"})"
    };
    const auto expectedResult7
    {
        R"({"data":{"checksum":"f25348b1ce5310f36c1ed859d13138fbb4e6bacb","inode":0,"item_id":"cbf2ac25a6775175f912ebf2abc72f6f51ab48ba","local_ip":"127.0.0.1","local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0},"operation":"INSERTED","type":"dbsync_ports"})"
    };
    const auto expectedResult8
    {
        R"({"data":{"checksum":"039934723aa69928b52e470c8d27365b0924b615","egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0},"operation":"INSERTED","type":"dbsync_processes"})"
    };
    const auto expectedResult9
    {
        R"({"data":{"architecture":"amd64","checksum":"c084f78ed87ed19974b1fd90bbf727c2d1416f7d","format":"deb","group":"x11","item_id":"4846c220a185b0fc251a07843efbfbb0d90ac4a5","location":" ","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14"},"operation":"INSERTED","type":"dbsync_packages"})"
    };
    const auto expectedResult10
    {
        R"({"data":{"checksum":"56162cd7bb632b4728ec868e8e271b01222ff131","hotfix":"KB12345678"},"operation":"INSERTED","type":"dbsync_hotfixes"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult7)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult8)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult9)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult10)).Times(1);

    InventoryConfig invConfig;
    invConfig.network = false;
    Configuration config(invConfig);
    Inventory::instance().setup(config);

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::instance().init(spInfoWrapper,
                                        callbackDataDelta,
                                        INVENTORY_DB_PATH,
                                        "",
                                        "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds(1));
    Inventory::instance().stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noPackages)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};
    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})")));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})")));
    EXPECT_CALL(*spInfoWrapper, networks()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"iface":[{"address":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "mac":"d4:5d:64:51:07:5d", "gateway":"192.168.0.1|600","broadcast":"127.255.255.255", "name":"ens1", "mtu":1500, "name":"enp4s0", "adapter":" ", "type":"ethernet", "state":"up", "dhcp":"disabled","iface":"Loopback Pseudo-Interface 1","metric":"75","netmask":"255.0.0.0","proto":"IPv4","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0, "IPv4":[{"address":"192.168.153.1","broadcast":"192.168.153.255","dhcp":"unknown","metric":" ","netmask":"255.255.255.0"}], "IPv6":[{"address":"fe80::250:56ff:fec0:8","dhcp":"unknown","metric":" ","netmask":"ffff:ffff:ffff:ffff::"}]}]})")));
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                   R"([{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0}])")));
    EXPECT_CALL(*spInfoWrapper, packages(_)).Times(0);

    EXPECT_CALL(*spInfoWrapper, hotfixes()).WillRepeatedly(Return(R"([{"hotfix":"KB12345678"}])"_json));

    EXPECT_CALL(*spInfoWrapper, processes(_))
    .Times(testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","scan_time":"2020/12/28 21:49:50", "nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0})"_json));

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);

            if (delta["type"].get_ref<const std::string&>().compare("dbsync_osinfo") == 0)
            {
                delta["data"].erase("checksum");
            }

            delta["data"].erase("scan_time");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"board_serial":"Intel Corporation","checksum":"af7b22eef8f5e06c04af4db49c9f8d1d28963918","cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","ram_free":2257872,"ram_total":4972208,"ram_usage":54},"operation":"INSERTED","type":"dbsync_hwinfo"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"architecture":"x86_64","hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"},"operation":"INSERTED","type":"dbsync_osinfo"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"adapter":" ","checksum":"165f7160ecd2838479ee4c43c1012b723736d90a","item_id":"25eef9a0a422a9b644fb6b73650453148bc6151c","mac":"d4:5d:64:51:07:5d","mtu":1500,"name":"enp4s0","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"state":"up","tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0,"type":"ethernet"},"operation":"INSERTED","type":"dbsync_network_iface"})"
    };
    const auto expectedResult4
    {
        R"({"data":{"checksum":"ff63981c231f110a0877ac6acd8862ac09877b5d","dhcp":"unknown","gateway":"192.168.0.1|600","iface":"enp4s0","item_id":"d633b040008ea38303d778431ee2fd0b4ee5a37a","metric":" ","type":"ipv4"},"operation":"INSERTED","type":"dbsync_network_protocol"})"
    };
    const auto expectedResult5
    {
        R"({"data":{"address":"192.168.153.1","broadcast":"192.168.153.255","checksum":"72dfd66759bd8062cdc17607d760a48c906189b3","iface":"enp4s0","item_id":"3d48ddc47fac84c62a19746af66fbfcf78547de9","netmask":"255.255.255.0","proto":0},"operation":"INSERTED","type":"dbsync_network_address"})"
    };
    const auto expectedResult6
    {
        R"({"data":{"address":"fe80::250:56ff:fec0:8","checksum":"f606d1a1c551874d8fab33e4e5cfaa0370673ec8","iface":"enp4s0","item_id":"65973316a5dc8615a6d20b2d6c4ce52ecd074496","netmask":"ffff:ffff:ffff:ffff::","proto":1},"operation":"INSERTED","type":"dbsync_network_address"})"
    };
    const auto expectedResult7
    {
        R"({"data":{"checksum":"f25348b1ce5310f36c1ed859d13138fbb4e6bacb","inode":0,"item_id":"cbf2ac25a6775175f912ebf2abc72f6f51ab48ba","local_ip":"127.0.0.1","local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0},"operation":"INSERTED","type":"dbsync_ports"})"
    };
    const auto expectedResult8
    {
        R"({"data":{"checksum":"039934723aa69928b52e470c8d27365b0924b615","egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0},"operation":"INSERTED","type":"dbsync_processes"})"
    };
    const auto expectedResult10
    {
        R"({"data":{"checksum":"56162cd7bb632b4728ec868e8e271b01222ff131","hotfix":"KB12345678"},"operation":"INSERTED","type":"dbsync_hotfixes"})"
    };
    const auto expectedResult11
    {
        R"({"data":{"checksum":"ea17673e7422c0ab04c4f1f111a5828be8cd366a","dhcp":"unknown","gateway":"192.168.0.1|600","iface":"enp4s0","item_id":"9dff246584835755137820c975f034d089e90b6f","metric":" ","type":"ipv6"},"operation":"INSERTED","type":"dbsync_network_protocol"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult4)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult5)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult6)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult7)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult8)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult10)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult11)).Times(1);

    InventoryConfig invConfig;
    invConfig.packages = false;
    Configuration config(invConfig);
    Inventory::instance().setup(config);

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::instance().init(spInfoWrapper,
                                          callbackDataDelta,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::instance().stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noPorts)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};
    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})")));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})")));
    EXPECT_CALL(*spInfoWrapper, networks()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"iface":[{"address":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "mac":"d4:5d:64:51:07:5d", "gateway":"192.168.0.1|600","broadcast":"127.255.255.255", "name":"ens1", "mtu":1500, "name":"enp4s0", "adapter":" ", "type":"ethernet", "state":"up", "dhcp":"disabled","iface":"Loopback Pseudo-Interface 1","metric":"75","netmask":"255.0.0.0","proto":"IPv4","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0, "IPv4":[{"address":"192.168.153.1","broadcast":"192.168.153.255","dhcp":"unknown","metric":" ","netmask":"255.255.255.0"}], "IPv6":[{"address":"fe80::250:56ff:fec0:8","dhcp":"unknown","metric":" ","netmask":"ffff:ffff:ffff:ffff::"}]}]})")));
    EXPECT_CALL(*spInfoWrapper, ports()).Times(0);
    EXPECT_CALL(*spInfoWrapper, packages(_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));

    EXPECT_CALL(*spInfoWrapper, hotfixes()).WillRepeatedly(Return(R"([{"hotfix":"KB12345678"}])"_json));

    EXPECT_CALL(*spInfoWrapper, processes(_))
    .Times(testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","scan_time":"2020/12/28 21:49:50", "nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0})"_json));

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);

            if (delta["type"].get_ref<const std::string&>().compare("dbsync_osinfo") == 0)
            {
                delta["data"].erase("checksum");
            }

            delta["data"].erase("scan_time");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"board_serial":"Intel Corporation","checksum":"af7b22eef8f5e06c04af4db49c9f8d1d28963918","cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","ram_free":2257872,"ram_total":4972208,"ram_usage":54},"operation":"INSERTED","type":"dbsync_hwinfo"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"architecture":"x86_64","hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"},"operation":"INSERTED","type":"dbsync_osinfo"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"adapter":" ","checksum":"165f7160ecd2838479ee4c43c1012b723736d90a","item_id":"25eef9a0a422a9b644fb6b73650453148bc6151c","mac":"d4:5d:64:51:07:5d","mtu":1500,"name":"enp4s0","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"state":"up","tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0,"type":"ethernet"},"operation":"INSERTED","type":"dbsync_network_iface"})"
    };
    const auto expectedResult4
    {
        R"({"data":{"checksum":"ff63981c231f110a0877ac6acd8862ac09877b5d","dhcp":"unknown","gateway":"192.168.0.1|600","iface":"enp4s0","item_id":"d633b040008ea38303d778431ee2fd0b4ee5a37a","metric":" ","type":"ipv4"},"operation":"INSERTED","type":"dbsync_network_protocol"})"
    };
    const auto expectedResult5
    {
        R"({"data":{"address":"192.168.153.1","broadcast":"192.168.153.255","checksum":"72dfd66759bd8062cdc17607d760a48c906189b3","iface":"enp4s0","item_id":"3d48ddc47fac84c62a19746af66fbfcf78547de9","netmask":"255.255.255.0","proto":0},"operation":"INSERTED","type":"dbsync_network_address"})"
    };
    const auto expectedResult6
    {
        R"({"data":{"address":"fe80::250:56ff:fec0:8","checksum":"f606d1a1c551874d8fab33e4e5cfaa0370673ec8","iface":"enp4s0","item_id":"65973316a5dc8615a6d20b2d6c4ce52ecd074496","netmask":"ffff:ffff:ffff:ffff::","proto":1},"operation":"INSERTED","type":"dbsync_network_address"})"
    };
    const auto expectedResult8
    {
        R"({"data":{"checksum":"039934723aa69928b52e470c8d27365b0924b615","egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0},"operation":"INSERTED","type":"dbsync_processes"})"
    };
    const auto expectedResult9
    {
        R"({"data":{"architecture":"amd64","checksum":"c084f78ed87ed19974b1fd90bbf727c2d1416f7d","format":"deb","group":"x11","item_id":"4846c220a185b0fc251a07843efbfbb0d90ac4a5","location":" ","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14"},"operation":"INSERTED","type":"dbsync_packages"})"
    };
    const auto expectedResult18
    {
        R"({"data":{"checksum":"56162cd7bb632b4728ec868e8e271b01222ff131","hotfix":"KB12345678"},"operation":"INSERTED","type":"dbsync_hotfixes"})"
    };
    const auto expectedResult20
    {
        R"({"data":{"checksum":"ea17673e7422c0ab04c4f1f111a5828be8cd366a","dhcp":"unknown","gateway":"192.168.0.1|600","iface":"enp4s0","item_id":"9dff246584835755137820c975f034d089e90b6f","metric":" ","type":"ipv6"},"operation":"INSERTED","type":"dbsync_network_protocol"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult4)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult5)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult6)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult8)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult9)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult18)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult20)).Times(1);

    InventoryConfig invConfig;
    invConfig.ports = false;
    invConfig.interval = 5;
    Configuration config(invConfig);
    Inventory::instance().setup(config);

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::instance().init(spInfoWrapper,
                                          callbackDataDelta,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds(1));
    Inventory::instance().stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noPortsAll)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};
    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})")));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})")));
    EXPECT_CALL(*spInfoWrapper, networks()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"iface":[{"address":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "mac":"d4:5d:64:51:07:5d", "gateway":"192.168.0.1|600","broadcast":"127.255.255.255", "name":"ens1", "mtu":1500, "name":"enp4s0", "adapter":" ", "type":"ethernet", "state":"up", "dhcp":"disabled","iface":"Loopback Pseudo-Interface 1","metric":"75","netmask":"255.0.0.0","proto":"IPv4","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0, "IPv4":[{"address":"192.168.153.1","broadcast":"192.168.153.255","dhcp":"unknown","metric":" ","netmask":"255.255.255.0"}], "IPv6":[{"address":"fe80::250:56ff:fec0:8","dhcp":"unknown","metric":" ","netmask":"ffff:ffff:ffff:ffff::"}]}]})")));
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                   R"([{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"udp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"","tx_queue":0},{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0}])")));
    EXPECT_CALL(*spInfoWrapper, packages(_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));

    EXPECT_CALL(*spInfoWrapper, hotfixes()).WillRepeatedly(Return(R"([{"hotfix":"KB12345678"}])"_json));

    EXPECT_CALL(*spInfoWrapper, processes(_))
    .Times(testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","scan_time":"2020/12/28 21:49:50", "nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0})"_json));

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);

            if (delta["type"].get_ref<const std::string&>().compare("dbsync_osinfo") == 0)
            {
                delta["data"].erase("checksum");
            }

            delta["data"].erase("scan_time");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"board_serial":"Intel Corporation","checksum":"af7b22eef8f5e06c04af4db49c9f8d1d28963918","cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","ram_free":2257872,"ram_total":4972208,"ram_usage":54},"operation":"INSERTED","type":"dbsync_hwinfo"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"architecture":"x86_64","hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"},"operation":"INSERTED","type":"dbsync_osinfo"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"adapter":" ","checksum":"165f7160ecd2838479ee4c43c1012b723736d90a","item_id":"25eef9a0a422a9b644fb6b73650453148bc6151c","mac":"d4:5d:64:51:07:5d","mtu":1500,"name":"enp4s0","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"state":"up","tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0,"type":"ethernet"},"operation":"INSERTED","type":"dbsync_network_iface"})"
    };
    const auto expectedResult4
    {
        R"({"data":{"checksum":"ff63981c231f110a0877ac6acd8862ac09877b5d","dhcp":"unknown","gateway":"192.168.0.1|600","iface":"enp4s0","item_id":"d633b040008ea38303d778431ee2fd0b4ee5a37a","metric":" ","type":"ipv4"},"operation":"INSERTED","type":"dbsync_network_protocol"})"
    };
    const auto expectedResult5
    {
        R"({"data":{"address":"192.168.153.1","broadcast":"192.168.153.255","checksum":"72dfd66759bd8062cdc17607d760a48c906189b3","iface":"enp4s0","item_id":"3d48ddc47fac84c62a19746af66fbfcf78547de9","netmask":"255.255.255.0","proto":0},"operation":"INSERTED","type":"dbsync_network_address"})"
    };
    const auto expectedResult6
    {
        R"({"data":{"address":"fe80::250:56ff:fec0:8","checksum":"f606d1a1c551874d8fab33e4e5cfaa0370673ec8","iface":"enp4s0","item_id":"65973316a5dc8615a6d20b2d6c4ce52ecd074496","netmask":"ffff:ffff:ffff:ffff::","proto":1},"operation":"INSERTED","type":"dbsync_network_address"})"
    };
    const auto expectedResult7
    {
        R"({"data":{"checksum":"f25348b1ce5310f36c1ed859d13138fbb4e6bacb","inode":0,"item_id":"cbf2ac25a6775175f912ebf2abc72f6f51ab48ba","local_ip":"127.0.0.1","local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0},"operation":"INSERTED","type":"dbsync_ports"})"
    };
    const auto expectedResult8
    {
        R"({"data":{"checksum":"039934723aa69928b52e470c8d27365b0924b615","egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0},"operation":"INSERTED","type":"dbsync_processes"})"
    };
    const auto expectedResult9
    {
        R"({"data":{"architecture":"amd64","checksum":"c084f78ed87ed19974b1fd90bbf727c2d1416f7d","format":"deb","group":"x11","item_id":"4846c220a185b0fc251a07843efbfbb0d90ac4a5","location":" ","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14"},"operation":"INSERTED","type":"dbsync_packages"})"
    };
    const auto expectedResult10
    {
        R"({"data":{"checksum":"56162cd7bb632b4728ec868e8e271b01222ff131","hotfix":"KB12345678"},"operation":"INSERTED","type":"dbsync_hotfixes"})"
    };
    const auto expectedResult11
    {
        R"({"data":{"checksum":"09d591fb0ed092c387f77b24af5bada43b5d519d","inode":0,"item_id":"7046b3f9cda975eb6567259c2469748e634dde49","local_ip":"127.0.0.1","local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"udp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":null,"tx_queue":0},"operation":"INSERTED","type":"dbsync_ports"})"
    };
    const auto expectedResult12
    {
        R"({"data":{"checksum":"ea17673e7422c0ab04c4f1f111a5828be8cd366a","dhcp":"unknown","gateway":"192.168.0.1|600","iface":"enp4s0","item_id":"9dff246584835755137820c975f034d089e90b6f","metric":" ","type":"ipv6"},"operation":"INSERTED","type":"dbsync_network_protocol"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult4)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult5)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult6)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult7)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult8)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult9)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult10)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult11)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult12)).Times(1);

    InventoryConfig invConfig;
    invConfig.portsAll = false;
    Configuration config(invConfig);
    Inventory::instance().setup(config);

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::instance().init(spInfoWrapper,
                                          callbackDataDelta,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds(1));
    Inventory::instance().stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noProcesses)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};
    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})")));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})")));
    EXPECT_CALL(*spInfoWrapper, networks()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"iface":[{"address":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "mac":"d4:5d:64:51:07:5d", "gateway":"192.168.0.1|600","broadcast":"127.255.255.255", "name":"ens1", "mtu":1500, "name":"enp4s0", "adapter":" ", "type":"ethernet", "state":"up", "dhcp":"disabled","iface":"Loopback Pseudo-Interface 1","metric":"75","netmask":"255.0.0.0","proto":"IPv4","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0, "IPv4":[{"address":"192.168.153.1","broadcast":"192.168.153.255","dhcp":"unknown","metric":" ","netmask":"255.255.255.0"}], "IPv6":[{"address":"fe80::250:56ff:fec0:8","dhcp":"unknown","metric":" ","netmask":"ffff:ffff:ffff:ffff::"}]}]})")));
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                   R"([{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0}])")));
    EXPECT_CALL(*spInfoWrapper, packages(_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));

    EXPECT_CALL(*spInfoWrapper, hotfixes()).WillRepeatedly(Return(R"([{"hotfix":"KB12345678"}])"_json));

    EXPECT_CALL(*spInfoWrapper, processes(_)).Times(0);

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);

            if (delta["type"].get_ref<const std::string&>().compare("dbsync_osinfo") == 0)
            {
                delta["data"].erase("checksum");
            }

            delta["data"].erase("scan_time");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"board_serial":"Intel Corporation","checksum":"af7b22eef8f5e06c04af4db49c9f8d1d28963918","cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","ram_free":2257872,"ram_total":4972208,"ram_usage":54},"operation":"INSERTED","type":"dbsync_hwinfo"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"architecture":"x86_64","hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"},"operation":"INSERTED","type":"dbsync_osinfo"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"adapter":" ","checksum":"165f7160ecd2838479ee4c43c1012b723736d90a","item_id":"25eef9a0a422a9b644fb6b73650453148bc6151c","mac":"d4:5d:64:51:07:5d","mtu":1500,"name":"enp4s0","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"state":"up","tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0,"type":"ethernet"},"operation":"INSERTED","type":"dbsync_network_iface"})"
    };
    const auto expectedResult4
    {
        R"({"data":{"checksum":"ff63981c231f110a0877ac6acd8862ac09877b5d","dhcp":"unknown","gateway":"192.168.0.1|600","iface":"enp4s0","item_id":"d633b040008ea38303d778431ee2fd0b4ee5a37a","metric":" ","type":"ipv4"},"operation":"INSERTED","type":"dbsync_network_protocol"})"
    };
    const auto expectedResult5
    {
        R"({"data":{"address":"192.168.153.1","broadcast":"192.168.153.255","checksum":"72dfd66759bd8062cdc17607d760a48c906189b3","iface":"enp4s0","item_id":"3d48ddc47fac84c62a19746af66fbfcf78547de9","netmask":"255.255.255.0","proto":0},"operation":"INSERTED","type":"dbsync_network_address"})"
    };
    const auto expectedResult6
    {
        R"({"data":{"address":"fe80::250:56ff:fec0:8","checksum":"f606d1a1c551874d8fab33e4e5cfaa0370673ec8","iface":"enp4s0","item_id":"65973316a5dc8615a6d20b2d6c4ce52ecd074496","netmask":"ffff:ffff:ffff:ffff::","proto":1},"operation":"INSERTED","type":"dbsync_network_address"})"
    };
    const auto expectedResult7
    {
        R"({"data":{"checksum":"f25348b1ce5310f36c1ed859d13138fbb4e6bacb","inode":0,"item_id":"cbf2ac25a6775175f912ebf2abc72f6f51ab48ba","local_ip":"127.0.0.1","local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0},"operation":"INSERTED","type":"dbsync_ports"})"
    };
    const auto expectedResult9
    {
        R"({"data":{"architecture":"amd64","checksum":"c084f78ed87ed19974b1fd90bbf727c2d1416f7d","format":"deb","group":"x11","item_id":"4846c220a185b0fc251a07843efbfbb0d90ac4a5","location":" ","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14"},"operation":"INSERTED","type":"dbsync_packages"})"
    };
    const auto expectedResult10
    {
        R"({"data":{"checksum":"56162cd7bb632b4728ec868e8e271b01222ff131","hotfix":"KB12345678"},"operation":"INSERTED","type":"dbsync_hotfixes"})"
    };
    const auto expectedResult11
    {
        R"({"data":{"checksum":"ea17673e7422c0ab04c4f1f111a5828be8cd366a","dhcp":"unknown","gateway":"192.168.0.1|600","iface":"enp4s0","item_id":"9dff246584835755137820c975f034d089e90b6f","metric":" ","type":"ipv6"},"operation":"INSERTED","type":"dbsync_network_protocol"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult4)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult5)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult6)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult7)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult9)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult10)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult11)).Times(1);

    InventoryConfig invConfig;
    invConfig.processes = false;
    Configuration config(invConfig);
    Inventory::instance().setup(config);

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::instance().init(spInfoWrapper,
                                          callbackDataDelta,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::instance().stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noHotfixes)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};
    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})")));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})")));
    EXPECT_CALL(*spInfoWrapper, networks()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"iface":[{"address":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "mac":"d4:5d:64:51:07:5d", "gateway":"192.168.0.1|600","broadcast":"127.255.255.255", "name":"ens1", "mtu":1500, "name":"enp4s0", "adapter":" ", "type":"ethernet", "state":"up", "dhcp":"disabled","iface":"Loopback Pseudo-Interface 1","metric":"75","netmask":"255.0.0.0","proto":"IPv4","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0, "IPv4":[{"address":"192.168.153.1","broadcast":"192.168.153.255","dhcp":"unknown","metric":" ","netmask":"255.255.255.0"}], "IPv6":[{"address":"fe80::250:56ff:fec0:8","dhcp":"unknown","metric":" ","netmask":"ffff:ffff:ffff:ffff::"}]}]})")));
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                   R"([{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0}])")));
    EXPECT_CALL(*spInfoWrapper, packages(_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));

    EXPECT_CALL(*spInfoWrapper, hotfixes()).Times(0);

    EXPECT_CALL(*spInfoWrapper, processes(_))
    .Times(testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","scan_time":"2020/12/28 21:49:50", "nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0})"_json));

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);

            if (delta["type"].get_ref<const std::string&>().compare("dbsync_osinfo") == 0)
            {
                delta["data"].erase("checksum");
            }

            delta["data"].erase("scan_time");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"board_serial":"Intel Corporation","checksum":"af7b22eef8f5e06c04af4db49c9f8d1d28963918","cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","ram_free":2257872,"ram_total":4972208,"ram_usage":54},"operation":"INSERTED","type":"dbsync_hwinfo"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"architecture":"x86_64","hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"},"operation":"INSERTED","type":"dbsync_osinfo"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"adapter":" ","checksum":"165f7160ecd2838479ee4c43c1012b723736d90a","item_id":"25eef9a0a422a9b644fb6b73650453148bc6151c","mac":"d4:5d:64:51:07:5d","mtu":1500,"name":"enp4s0","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"state":"up","tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0,"type":"ethernet"},"operation":"INSERTED","type":"dbsync_network_iface"})"
    };
    const auto expectedResult4
    {
        R"({"data":{"checksum":"ff63981c231f110a0877ac6acd8862ac09877b5d","dhcp":"unknown","gateway":"192.168.0.1|600","iface":"enp4s0","item_id":"d633b040008ea38303d778431ee2fd0b4ee5a37a","metric":" ","type":"ipv4"},"operation":"INSERTED","type":"dbsync_network_protocol"})"
    };
    const auto expectedResult5
    {
        R"({"data":{"address":"192.168.153.1","broadcast":"192.168.153.255","checksum":"72dfd66759bd8062cdc17607d760a48c906189b3","iface":"enp4s0","item_id":"3d48ddc47fac84c62a19746af66fbfcf78547de9","netmask":"255.255.255.0","proto":0},"operation":"INSERTED","type":"dbsync_network_address"})"
    };
    const auto expectedResult6
    {
        R"({"data":{"address":"fe80::250:56ff:fec0:8","checksum":"f606d1a1c551874d8fab33e4e5cfaa0370673ec8","iface":"enp4s0","item_id":"65973316a5dc8615a6d20b2d6c4ce52ecd074496","netmask":"ffff:ffff:ffff:ffff::","proto":1},"operation":"INSERTED","type":"dbsync_network_address"})"
    };
    const auto expectedResult7
    {
        R"({"data":{"checksum":"f25348b1ce5310f36c1ed859d13138fbb4e6bacb","inode":0,"item_id":"cbf2ac25a6775175f912ebf2abc72f6f51ab48ba","local_ip":"127.0.0.1","local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0},"operation":"INSERTED","type":"dbsync_ports"})"
    };
    const auto expectedResult8
    {
        R"({"data":{"checksum":"039934723aa69928b52e470c8d27365b0924b615","egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0},"operation":"INSERTED","type":"dbsync_processes"})"
    };
    const auto expectedResult9
    {
        R"({"data":{"architecture":"amd64","checksum":"c084f78ed87ed19974b1fd90bbf727c2d1416f7d","format":"deb","group":"x11","item_id":"4846c220a185b0fc251a07843efbfbb0d90ac4a5","location":" ","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14"},"operation":"INSERTED","type":"dbsync_packages"})"
    };
    const auto expectedResult18
    {
        R"({"data":{"checksum":"ea17673e7422c0ab04c4f1f111a5828be8cd366a","dhcp":"unknown","gateway":"192.168.0.1|600","iface":"enp4s0","item_id":"9dff246584835755137820c975f034d089e90b6f","metric":" ","type":"ipv6"},"operation":"INSERTED","type":"dbsync_network_protocol"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult4)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult5)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult6)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult7)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult8)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult9)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult18)).Times(1);

    InventoryConfig invConfig;
    invConfig.hotfixes = false;
    Configuration config(invConfig);
    Inventory::instance().setup(config);

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::instance().init(spInfoWrapper,
                                          callbackDataDelta,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::instance().stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, scanInvalidData)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};
    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","ram_free":2257872,"ram_total":4972208,"ram_usage":54})")));
    EXPECT_CALL(*spInfoWrapper, networks()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"iface":[{"address":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "mac":"d4:5d:64:51:07:5d", "gateway":"192.168.0.1|600","broadcast":"127.255.255.255", "name":"ens1", "mtu":1500, "name":"enp4s0", "adapter":" ", "type":"ethernet", "state":"up", "dhcp":"disabled","iface":"Loopback Pseudo-Interface 1","metric":"75","netmask":"255.0.0.0","proto":"IPv4","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0, "IPv4":[{"address":"192.168.153.1","broadcast":"192.168.153.255","dhcp":"unknown","metric":" ","netmask":"255.255.255.0"}], "IPv6":[{"address":"fe80::250:56ff:fec0:8","dhcp":"unknown","metric":" ","netmask":"ffff:ffff:ffff:ffff::"}]}]})")));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})")));
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                   R"([{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0}])")));
    EXPECT_CALL(*spInfoWrapper, hotfixes()).WillRepeatedly(Return(R"([{"hotfix":"KB12345678"},{"hotfix":"KB87654321"}])"_json));
    EXPECT_CALL(*spInfoWrapper, packages(_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"name":"TEXT", "scan_time":"2020/12/28 21:49:50", "version":"TEXT", "vendor":"TEXT", "install_time":"TEXT", "location":"TEXT", "architecture":"TEXT", "groups":"TEXT", "description":"TEXT", "size":"TEXT", "priority":"TEXT", "multiarch":"TEXT", "source":"TEXT", "os_patch":"TEXT"})"_json));
    EXPECT_CALL(*spInfoWrapper, processes(_))
    .Times(testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","scan_time":"2020/12/28 21:49:50", "nice":0,"nlwp":1,"pgrp":0,"pid":431625,"ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0})"_json));

    InventoryConfig invConfig;
    invConfig.interval = 60;
    Configuration config(invConfig);
    Inventory::instance().setup(config);

    std::thread t
    {
        [&spInfoWrapper]()
        {
            Inventory::instance().init(spInfoWrapper,
                                          reportFunction,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
        }
    };
    std::this_thread::sleep_for(std::chrono::seconds{1});
    Inventory::instance().stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, portAllEnable)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(R"(
    [
        {
            "inode":43481,
            "local_ip":"0.0.0.0",
            "local_port":47748,
            "pid":0,
            "process_name":"",
            "protocol":"udp",
            "remote_ip":"0.0.0.0",
            "remote_port":0,
            "rx_queue":0,
            "state":"",
            "tx_queue":0
        },
        {
            "inode":43482,
            "local_ip":"::",
            "local_port":51087,
            "pid":0,
            "process_name":"",
            "protocol":"udp6",
            "remote_ip":"::",
            "remote_port":0,
            "rx_queue":0,
            "state":"",
            "tx_queue":0
        },
        {
            "inode":50324,
            "local_ip":"127.0.0.1",
            "local_port":33060,
            "pid":0,
            "process_name":"",
            "protocol":"tcp",
            "remote_ip":"0.0.0.0",
            "remote_port":0,
            "rx_queue":0,
            "state":"listening",
            "tx_queue":0
        },
        {
            "inode":122575,
            "local_ip":"192.168.0.104",
            "local_port":39106,
            "pid":0,
            "process_name":"",
            "protocol":"tcp",
            "remote_ip":"44.238.116.130",
            "remote_port":443,
            "rx_queue":0,
            "state":"established",
            "tx_queue":0
        },
        {
            "inode":122575,
            "local_ip":"192.168.0.104",
            "local_port":39106,
            "pid":0,
            "process_name":"",
            "protocol":"tcp",
            "remote_ip":"44.238.116.130",
            "remote_port":443,
            "rx_queue":0,
            "state":"established",
            "tx_queue":0
        }
    ])")));

    CallbackMock wrapper;
    std::function<void(const std::string&)> callbackData
    {
        [&wrapper](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("checksum");
            delta["data"].erase("scan_time");
            wrapper.callbackMock(delta.dump());
        }
    };
    const auto expectedResult1
    {
        R"({"data":{"inode":43481,"item_id":"12903a43db24ab10d872547cdd1d786a5876a0da","local_ip":"0.0.0.0","local_port":47748,"pid":0,"process_name":null,"protocol":"udp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":null,"tx_queue":0},"operation":"INSERTED","type":"dbsync_ports"})"
    };

    const auto expectedResult2
    {
        R"({"data":{"inode":43482,"item_id":"ca7c9aff241cb251c6ad31e30b806366ecb2ad5f","local_ip":"::","local_port":51087,"pid":0,"process_name":null,"protocol":"udp6","remote_ip":"::","remote_port":0,"rx_queue":0,"state":null,"tx_queue":0},"operation":"INSERTED","type":"dbsync_ports"})"
    };

    const auto expectedResult3
    {
        R"({"data":{"inode":50324,"item_id":"8c790ef53962dd27f4516adb1d7f3f6096bc6d29","local_ip":"127.0.0.1","local_port":33060,"pid":0,"process_name":null,"protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0},"operation":"INSERTED","type":"dbsync_ports"})"
    };

    const auto expectedResult4
    {
        R"({"data":{"inode":122575,"item_id":"d5511242275bd3f2d57175f248108d6c3b39c438","local_ip":"192.168.0.104","local_port":39106,"pid":0,"process_name":null,"protocol":"tcp","remote_ip":"44.238.116.130","remote_port":443,"rx_queue":0,"state":"established","tx_queue":0},"operation":"INSERTED","type":"dbsync_ports"})"
    };

    EXPECT_CALL(wrapper, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapper, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapper, callbackMock(expectedResult3)).Times(1);
    EXPECT_CALL(wrapper, callbackMock(expectedResult4)).Times(1);

    InventoryConfig invConfig;
    invConfig.hardware = false;
    invConfig.os = false;
    invConfig.network = false;
    invConfig.packages = false;
    invConfig.processes = false;
    invConfig.hotfixes = false;
    Configuration config(invConfig);
    Inventory::instance().setup(config);

    std::thread t
    {
        [&spInfoWrapper, &callbackData]()
        {
            Inventory::instance().init(spInfoWrapper,
                                          callbackData,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::instance().stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, portAllDisable)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(R"(
    [
        {
            "inode":43481,
            "local_ip":"0.0.0.0",
            "local_port":47748,
            "pid":0,
            "process_name":"",
            "protocol":"udp",
            "remote_ip":"0.0.0.0",
            "remote_port":0,
            "rx_queue":0,
            "state":"",
            "tx_queue":0
        },
        {
            "inode":43482,
            "local_ip":"::",
            "local_port":51087,
            "pid":0,
            "process_name":"",
            "protocol":"udp6",
            "remote_ip":"::",
            "remote_port":0,
            "rx_queue":0,
            "state":"",
            "tx_queue":0
        },
        {
            "inode":50324,
            "local_ip":"127.0.0.1",
            "local_port":33060,
            "pid":0,
            "process_name":"",
            "protocol":"tcp",
            "remote_ip":"0.0.0.0",
            "remote_port":0,
            "rx_queue":0,
            "state":"listening",
            "tx_queue":0
        },
        {
            "inode":50324,
            "local_ip":"127.0.0.1",
            "local_port":33060,
            "pid":0,
            "process_name":"",
            "protocol":"tcp",
            "remote_ip":"0.0.0.0",
            "remote_port":0,
            "rx_queue":0,
            "state":"listening",
            "tx_queue":0
        },
        {
            "inode":122575,
            "local_ip":"192.168.0.104",
            "local_port":39106,
            "pid":0,
            "process_name":"",
            "protocol":"tcp",
            "remote_ip":"44.238.116.130",
            "remote_port":443,
            "rx_queue":0,
            "state":"established",
            "tx_queue":0
        }
    ])")));

    CallbackMock wrapper;
    std::function<void(const std::string&)> callbackData
    {
        [&wrapper](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("checksum");
            delta["data"].erase("scan_time");
            wrapper.callbackMock(delta.dump());
        }
    };
    const auto expectedResult1
    {
        R"({"data":{"inode":43481,"item_id":"12903a43db24ab10d872547cdd1d786a5876a0da","local_ip":"0.0.0.0","local_port":47748,"pid":0,"process_name":null,"protocol":"udp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":null,"tx_queue":0},"operation":"INSERTED","type":"dbsync_ports"})"
    };

    const auto expectedResult2
    {
        R"({"data":{"inode":43482,"item_id":"ca7c9aff241cb251c6ad31e30b806366ecb2ad5f","local_ip":"::","local_port":51087,"pid":0,"process_name":null,"protocol":"udp6","remote_ip":"::","remote_port":0,"rx_queue":0,"state":null,"tx_queue":0},"operation":"INSERTED","type":"dbsync_ports"})"
    };

    const auto expectedResult3
    {
        R"({"data":{"inode":50324,"item_id":"8c790ef53962dd27f4516adb1d7f3f6096bc6d29","local_ip":"127.0.0.1","local_port":33060,"pid":0,"process_name":null,"protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0},"operation":"INSERTED","type":"dbsync_ports"})"
    };

    EXPECT_CALL(wrapper, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapper, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapper, callbackMock(expectedResult3)).Times(1);

    InventoryConfig invConfig;
    invConfig.hardware = false;
    invConfig.os = false;
    invConfig.network = false;
    invConfig.packages = false;
    invConfig.portsAll = false;
    invConfig.processes = false;
    invConfig.hotfixes = false;
    Configuration config(invConfig);
    Inventory::instance().setup(config);

    std::thread t
    {
        [&spInfoWrapper, &callbackData]()
        {
            Inventory::instance().init(spInfoWrapper,
                                          callbackData,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::instance().stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, PackagesDuplicated)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, packages(_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::DoAll(
                  ::testing::InvokeArgument<0>
                  (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json),
                  ::testing::InvokeArgument<0>
                  (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json)));



    CallbackMock wrapper;
    std::function<void(const std::string&)> callbackData
    {
        [&wrapper](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("checksum");
            delta["data"].erase("scan_time");
            wrapper.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"architecture":"amd64","format":"deb","group":"x11","item_id":"4846c220a185b0fc251a07843efbfbb0d90ac4a5","location":" ","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14"},"operation":"INSERTED","type":"dbsync_packages"})"
    };

    EXPECT_CALL(wrapper, callbackMock(expectedResult1)).Times(1);

    InventoryConfig invConfig;
    invConfig.hardware = false;
    invConfig.os = false;
    invConfig.network = false;
    invConfig.ports = false;
    invConfig.portsAll = false;
    invConfig.processes = false;
    invConfig.hotfixes = false;
    Configuration config(invConfig);
    Inventory::instance().setup(config);

    std::thread t
    {
        [&spInfoWrapper, &callbackData]()
        {
            Inventory::instance().init(spInfoWrapper,
                                          callbackData,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::instance().stop();

    if (t.joinable())
    {
        t.join();
    }
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}