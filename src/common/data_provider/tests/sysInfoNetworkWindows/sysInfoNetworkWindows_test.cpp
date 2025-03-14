/*
 * Wazuh SysInfo
 * Copyright (C) 2015, Wazuh Inc.
 * October 19, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "sysInfoNetworkWindows_test.h"
#include "network/networkFamilyDataAFactory.h"
#include "network/networkInterfaceWindows.h"
#include "networkWindowsHelper.hpp"

void SysInfoNetworkWindowsTest::SetUp() {};

void SysInfoNetworkWindowsTest::TearDown() {};

using ::testing::_;
using ::testing::Return;

class SysInfoNetworkWindowsWrapperMock : public INetworkInterfaceWrapper
{
public:
    SysInfoNetworkWindowsWrapperMock() = default;
    virtual ~SysInfoNetworkWindowsWrapperMock() = default;
    MOCK_METHOD(int, family, (), (const override));
    MOCK_METHOD(std::string, name, (), (const override));
    MOCK_METHOD(void, adapter, (nlohmann::json&), (const override));
    MOCK_METHOD(std::string, address, (), (const override));
    MOCK_METHOD(std::string, netmask, (), (const override));
    MOCK_METHOD(void, broadcast, (nlohmann::json&), (const override));
    MOCK_METHOD(std::string, addressV6, (), (const override));
    MOCK_METHOD(std::string, netmaskV6, (), (const override));
    MOCK_METHOD(void, broadcastV6, (nlohmann::json&), (const override));
    MOCK_METHOD(void, gateway, (nlohmann::json&), (const override));
    MOCK_METHOD(void, metrics, (nlohmann::json&), (const override));
    MOCK_METHOD(void, metricsV6, (nlohmann::json&), (const override));
    MOCK_METHOD(void, dhcp, (nlohmann::json&), (const override));
    MOCK_METHOD(void, mtu, (nlohmann::json&), (const override));
    MOCK_METHOD(LinkStats, stats, (), (const override));
    MOCK_METHOD(void, type, (nlohmann::json&), (const override));
    MOCK_METHOD(void, state, (nlohmann::json&), (const override));
    MOCK_METHOD(void, MAC, (nlohmann::json&), (const override));
};

TEST_F(SysInfoNetworkWindowsTest, Test_IPV4_THROW)
{
    auto mock {std::make_shared<SysInfoNetworkWindowsWrapperMock>()};
    nlohmann::json networkInfo {};
    EXPECT_CALL(*mock, family()).Times(1).WillOnce(Return(Utils::IPV4));
    EXPECT_CALL(*mock, address()).Times(1).WillOnce(Return(""));
    EXPECT_ANY_THROW(FactoryNetworkFamilyCreator<OSPlatformType::WINDOWS>::create(mock)->buildNetworkData(networkInfo));
}

TEST_F(SysInfoNetworkWindowsTest, Test_IPV6_THROW)
{
    auto mock {std::make_shared<SysInfoNetworkWindowsWrapperMock>()};
    nlohmann::json networkInfo {};
    EXPECT_CALL(*mock, family()).Times(1).WillOnce(Return(Utils::IPV6));
    EXPECT_CALL(*mock, addressV6()).Times(1).WillOnce(Return(""));
    EXPECT_ANY_THROW(FactoryNetworkFamilyCreator<OSPlatformType::WINDOWS>::create(mock)->buildNetworkData(networkInfo));
}

TEST_F(SysInfoNetworkWindowsTest, Test_AF_UNSPEC_THROW_NULLPTR)
{
    nlohmann::json networkInfo {};
    EXPECT_ANY_THROW(
        FactoryNetworkFamilyCreator<OSPlatformType::WINDOWS>::create(nullptr)->buildNetworkData(networkInfo));
}

TEST_F(SysInfoNetworkWindowsTest, Test_IPV4)
{
    auto mock {std::make_shared<SysInfoNetworkWindowsWrapperMock>()};
    nlohmann::json networkInfo {};
    const std::string address {"192.168.0.1"};
    const std::string netmask {"255.255.255.0"};
    const std::string broadcast {"192.168.0.255"};
    const std::string dhcp {"8.8.8.8"};
    const std::string metrics {"25"};
    EXPECT_CALL(*mock, family()).Times(1).WillOnce(Return(Utils::IPV4));
    EXPECT_CALL(*mock, address()).Times(1).WillOnce(Return(address));
    EXPECT_CALL(*mock, netmask()).Times(1).WillOnce(Return(netmask));
    EXPECT_CALL(*mock, broadcast(_))
        .Times(1)
        .WillOnce(testing::Invoke([&](nlohmann::json& jsonOut) { jsonOut["broadcast"] = broadcast; }));
    EXPECT_CALL(*mock, dhcp(_))
        .Times(1)
        .WillOnce(testing::Invoke([&](nlohmann::json& jsonOut) { jsonOut["dhcp"] = dhcp; }));
    EXPECT_CALL(*mock, metrics(_))
        .Times(1)
        .WillOnce(testing::Invoke([&](nlohmann::json& jsonOut) { jsonOut["metric"] = metrics; }));
    EXPECT_NO_THROW(FactoryNetworkFamilyCreator<OSPlatformType::WINDOWS>::create(mock)->buildNetworkData(networkInfo));

    for (auto& element : networkInfo.at("IPv4"))
    {
        EXPECT_EQ(address, element.at("address").get_ref<const std::string&>());
        EXPECT_EQ(netmask, element.at("netmask").get_ref<const std::string&>());
        EXPECT_EQ(broadcast, element.at("broadcast").get_ref<const std::string&>());
        EXPECT_EQ(dhcp, element.at("dhcp").get_ref<const std::string&>());
        EXPECT_EQ(metrics, element.at("metric").get_ref<const std::string&>());
    }
}

TEST_F(SysInfoNetworkWindowsTest, Test_IPV6)
{
    auto mock {std::make_shared<SysInfoNetworkWindowsWrapperMock>()};
    nlohmann::json networkInfo {};
    const std::string address {"2001:db8:85a3:8d3:1319:8a2e:370:7348"};
    const std::string netmask {"2001:db8:abcd:0012:ffff:ffff:ffff:ffff"};
    const std::string broadcast {"2001:db8:85a3:8d3:1319:8a2e:370:0000"};
    const std::string dhcp {"8.8.8.8"};
    const std::string metrics {"25"};
    EXPECT_CALL(*mock, family()).Times(1).WillOnce(Return(Utils::IPV6));
    EXPECT_CALL(*mock, addressV6()).Times(1).WillOnce(Return(address));
    EXPECT_CALL(*mock, netmaskV6()).Times(1).WillOnce(Return(netmask));
    EXPECT_CALL(*mock, broadcastV6(_))
        .Times(1)
        .WillOnce(testing::Invoke([&](nlohmann::json& jsonOut) { jsonOut["broadcast"] = broadcast; }));
    EXPECT_CALL(*mock, dhcp(_))
        .Times(1)
        .WillOnce(testing::Invoke([&](nlohmann::json& jsonOut) { jsonOut["dhcp"] = dhcp; }));
    EXPECT_CALL(*mock, metricsV6(_))
        .Times(1)
        .WillOnce(testing::Invoke([&](nlohmann::json& jsonOut) { jsonOut["metric"] = metrics; }));
    EXPECT_NO_THROW(FactoryNetworkFamilyCreator<OSPlatformType::WINDOWS>::create(mock)->buildNetworkData(networkInfo));

    for (auto& element : networkInfo.at("IPv6"))
    {
        EXPECT_EQ(address, element.at("address").get_ref<const std::string&>());
        EXPECT_EQ(netmask, element.at("netmask").get_ref<const std::string&>());
        EXPECT_EQ(broadcast, element.at("broadcast").get_ref<const std::string&>());
        EXPECT_EQ(dhcp, element.at("dhcp").get_ref<const std::string&>());
        EXPECT_EQ(metrics, element.at("metric").get_ref<const std::string&>());
    }
}

TEST_F(SysInfoNetworkWindowsTest, Test_COMMON_DATA)
{
    auto mock {std::make_shared<SysInfoNetworkWindowsWrapperMock>()};
    nlohmann::json networkInfo {};
    const std::string name {"eth01"};
    const std::string type {"2001:db8:abcd:0012:ffff:ffff:ffff:ffff"};
    const std::string state {"up"};
    const std::string MAC {"00:A0:C9:14:C8:29"};
    const uint32_t mtu {1500};
    const std::string gateway {"10.2.2.50"};
    EXPECT_CALL(*mock, family()).Times(1).WillOnce(Return(Utils::COMMON_DATA));
    EXPECT_CALL(*mock, name()).Times(1).WillOnce(Return(name));
    EXPECT_CALL(*mock, type(_))
        .Times(1)
        .WillOnce(testing::Invoke([&](nlohmann::json& jsonOut) { jsonOut["type"] = type; }));
    EXPECT_CALL(*mock, state(_))
        .Times(1)
        .WillOnce(testing::Invoke([&](nlohmann::json& jsonOut) { jsonOut["state"] = state; }));
    EXPECT_CALL(*mock, MAC(_))
        .Times(1)
        .WillOnce(testing::Invoke([&](nlohmann::json& jsonOut) { jsonOut["mac"] = MAC; }));
    EXPECT_CALL(*mock, stats()).Times(1).WillOnce(Return(LinkStats {0, 1, 2, 3, 4, 5, 6, 7}));
    EXPECT_CALL(*mock, mtu(_))
        .Times(1)
        .WillOnce(testing::Invoke([&](nlohmann::json& jsonOut) { jsonOut["mtu"] = mtu; }));
    EXPECT_CALL(*mock, gateway(_))
        .Times(1)
        .WillOnce(testing::Invoke([&](nlohmann::json& jsonOut) { jsonOut["gateway"] = gateway; }));

    EXPECT_NO_THROW(FactoryNetworkFamilyCreator<OSPlatformType::WINDOWS>::create(mock)->buildNetworkData(networkInfo));

    EXPECT_EQ(name, networkInfo.at("name").get_ref<const std::string&>());
    EXPECT_EQ(type, networkInfo.at("type").get_ref<const std::string&>());
    EXPECT_EQ(state, networkInfo.at("state").get_ref<const std::string&>());
    EXPECT_EQ(MAC, networkInfo.at("mac").get_ref<const std::string&>());

    EXPECT_EQ(1, networkInfo.at("tx_packets").get<int32_t>());
    EXPECT_EQ(0, networkInfo.at("rx_packets").get<int32_t>());
    EXPECT_EQ(3, networkInfo.at("tx_bytes").get<int32_t>());
    EXPECT_EQ(2, networkInfo.at("rx_bytes").get<int32_t>());
    EXPECT_EQ(5, networkInfo.at("tx_errors").get<int32_t>());
    EXPECT_EQ(4, networkInfo.at("rx_errors").get<int32_t>());
    EXPECT_EQ(7, networkInfo.at("tx_dropped").get<int32_t>());
    EXPECT_EQ(6, networkInfo.at("rx_dropped").get<int32_t>());

    EXPECT_EQ(mtu, networkInfo.at("mtu").get<uint32_t>());
    EXPECT_EQ(gateway, networkInfo.at("gateway").get_ref<const std::string&>());
}
