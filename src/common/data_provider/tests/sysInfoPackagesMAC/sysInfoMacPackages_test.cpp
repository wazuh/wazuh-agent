/*
 * Wazuh SysInfo
 * Copyright (C) 2015, Wazuh Inc.
 * December 14, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "sysInfoMacPackages_test.h"
#include "packages/macportsWrapper.h"
#include "packages/packageMac.h"
#include "sqliteWrapperTemp.hpp"
#include "sqliteWrapperTempMock.h"

void SysInfoMacPackagesTest::SetUp() {};

void SysInfoMacPackagesTest::TearDown() {};

using ::testing::_;
using ::testing::An;
using ::testing::ByMove;
using ::testing::Return;

class SysInfoMacPackagesWrapperMock : public IPackageWrapper
{
public:
    SysInfoMacPackagesWrapperMock() = default;
    virtual ~SysInfoMacPackagesWrapperMock() = default;
    MOCK_METHOD(void, name, (nlohmann::json & package), (const override));
    MOCK_METHOD(void, version, (nlohmann::json & package), (const override));
    MOCK_METHOD(void, groups, (nlohmann::json & package), (const override));
    MOCK_METHOD(void, description, (nlohmann::json & package), (const override));
    MOCK_METHOD(void, architecture, (nlohmann::json & package), (const override));
    MOCK_METHOD(void, format, (nlohmann::json & package), (const override));
    MOCK_METHOD(void, osPatch, (nlohmann::json & package), (const override));
    MOCK_METHOD(void, source, (nlohmann::json & package), (const override));
    MOCK_METHOD(void, location, (nlohmann::json & package), (const override));
    MOCK_METHOD(void, priority, (nlohmann::json & package), (const override));
    MOCK_METHOD(void, size, (nlohmann::json & package), (const override));
    MOCK_METHOD(void, vendor, (nlohmann::json & package), (const override));
    MOCK_METHOD(void, install_time, (nlohmann::json & package), (const override));
    MOCK_METHOD(void, multiarch, (nlohmann::json & package), (const override));
};

TEST_F(SysInfoMacPackagesTest, Test_SPEC_Data)
{
    auto mock {std::make_shared<SysInfoMacPackagesWrapperMock>()};
    nlohmann::json packages {};
    EXPECT_CALL(*mock, name(_)).WillOnce([](nlohmann::json& package) { package["name"] = "1"; });
    EXPECT_CALL(*mock, version(_)).WillOnce([](nlohmann::json& package) { package["version"] = "2"; });
    EXPECT_CALL(*mock, groups(_)).WillOnce([](nlohmann::json& package) { package["groups"] = "3"; });
    EXPECT_CALL(*mock, description(_)).WillOnce([](nlohmann::json& package) { package["description"] = "4"; });
    EXPECT_CALL(*mock, architecture(_)).WillOnce([](nlohmann::json& package) { package["architecture"] = "5"; });
    EXPECT_CALL(*mock, format(_)).WillOnce([](nlohmann::json& package) { package["format"] = "6"; });
    EXPECT_CALL(*mock, source(_)).WillOnce([](nlohmann::json& package) { package["source"] = "7"; });
    EXPECT_CALL(*mock, location(_)).WillOnce([](nlohmann::json& package) { package["location"] = "8"; });
    EXPECT_CALL(*mock, priority(_)).WillOnce([](nlohmann::json& package) { package["priority"] = "9"; });
    EXPECT_CALL(*mock, size(_)).WillOnce([](nlohmann::json& package) { package["size"] = 10; });
    EXPECT_CALL(*mock, vendor(_)).WillOnce([](nlohmann::json& package) { package["vendor"] = "11"; });
    EXPECT_CALL(*mock, install_time(_)).WillOnce([](nlohmann::json& package) { package["install_time"] = "12"; });
    EXPECT_CALL(*mock, multiarch(_)).WillOnce([](nlohmann::json& package) { package["multiarch"] = "13"; });

    EXPECT_NO_THROW(std::make_unique<BSDPackageImpl>(mock)->buildPackageData(packages));
    EXPECT_EQ("1", packages.at("name").get_ref<const std::string&>());
    EXPECT_EQ("2", packages.at("version").get_ref<const std::string&>());
    EXPECT_EQ("3", packages.at("groups").get_ref<const std::string&>());
    EXPECT_EQ("4", packages.at("description").get_ref<const std::string&>());
    EXPECT_EQ("5", packages.at("architecture").get_ref<const std::string&>());
    EXPECT_EQ("6", packages.at("format").get_ref<const std::string&>());
    EXPECT_EQ("7", packages.at("source").get_ref<const std::string&>());
    EXPECT_EQ("8", packages.at("location").get_ref<const std::string&>());
    EXPECT_EQ("9", packages.at("priority").get_ref<const std::string&>());
    EXPECT_EQ(10, packages.at("size").get<const int>());
    EXPECT_EQ("11", packages.at("vendor").get_ref<const std::string&>());
    EXPECT_EQ("12", packages.at("install_time").get_ref<const std::string&>());
    EXPECT_EQ("13", packages.at("multiarch").get_ref<const std::string&>());
}

TEST_F(SysInfoMacPackagesTest, macPortsValidData)
{
    auto mockStatement {std::make_unique<MockStatement>()};
    EXPECT_CALL(*mockStatement, columnsCount()).WillOnce(Return(5));

    auto mockColumn_1 {std::make_unique<MockColumn>()};
    EXPECT_CALL(*mockColumn_1, value(An<const std::string&>())).WillOnce(Return("neovim"));
    auto mockColumn_2 {std::make_unique<MockColumn>()};
    EXPECT_CALL(*mockColumn_2, value(An<const std::string&>())).WillOnce(Return("0.8.1"));
    auto mockColumn_3 {std::make_unique<MockColumn>()};
    EXPECT_CALL(*mockColumn_3, value(An<const int64_t&>())).WillOnce(Return(1690831043));
    auto mockColumn_4 {std::make_unique<MockColumn>()};
    EXPECT_CALL(*mockColumn_4, value(An<const std::string&>()))
        .WillOnce(Return("/opt/local/var/macports/software/neovim/neovim-0.8.1.tgz"));
    auto mockColumn_5 {std::make_unique<MockColumn>()};
    EXPECT_CALL(*mockColumn_5, value(An<const std::string&>())).WillOnce(Return("x86_64"));

    EXPECT_CALL(*mockColumn_1, hasValue()).WillOnce(Return(true));
    EXPECT_CALL(*mockColumn_2, hasValue()).WillOnce(Return(true));
    EXPECT_CALL(*mockColumn_3, hasValue()).WillOnce(Return(true));
    EXPECT_CALL(*mockColumn_4, hasValue()).WillOnce(Return(true));
    EXPECT_CALL(*mockColumn_5, hasValue()).WillOnce(Return(true));

    EXPECT_CALL(*mockStatement, column(0)).WillOnce(Return(ByMove(std::move(mockColumn_1))));
    EXPECT_CALL(*mockStatement, column(1)).WillOnce(Return(ByMove(std::move(mockColumn_2))));
    EXPECT_CALL(*mockStatement, column(2)).WillOnce(Return(ByMove(std::move(mockColumn_3))));
    EXPECT_CALL(*mockStatement, column(3)).WillOnce(Return(ByMove(std::move(mockColumn_4))));
    EXPECT_CALL(*mockStatement, column(4)).WillOnce(Return(ByMove(std::move(mockColumn_5))));

    MacportsWrapper macportsMock(*mockStatement);

    nlohmann::json package;
    macportsMock.name(package);
    macportsMock.version(package);
    macportsMock.install_time(package);
    macportsMock.location(package);
    macportsMock.architecture(package);

    EXPECT_EQ(package["name"], "neovim");
    EXPECT_EQ(package["version"], "0.8.1");
    EXPECT_FALSE(package["install_time"].empty());
    EXPECT_EQ(package["location"], "/opt/local/var/macports/software/neovim/neovim-0.8.1.tgz");
    EXPECT_EQ(package["architecture"], "x86_64");
}

TEST_F(SysInfoMacPackagesTest, macPortsValidDataEmptyFields)
{
    auto mockStatement {std::make_unique<MockStatement>()};
    EXPECT_CALL(*mockStatement, columnsCount()).WillOnce(Return(5));

    auto mockColumn_1 {std::make_unique<MockColumn>()};
    EXPECT_CALL(*mockColumn_1, value(An<const std::string&>())).WillOnce(Return("neovim"));
    auto mockColumn_2 {std::make_unique<MockColumn>()};
    EXPECT_CALL(*mockColumn_2, value(An<const std::string&>())).WillOnce(Return(""));
    auto mockColumn_3 {std::make_unique<MockColumn>()};
    EXPECT_CALL(*mockColumn_3, value(An<const int64_t&>())).WillOnce(Return(0));
    auto mockColumn_4 {std::make_unique<MockColumn>()};
    EXPECT_CALL(*mockColumn_4, value(An<const std::string&>())).WillOnce(Return(""));
    auto mockColumn_5 {std::make_unique<MockColumn>()};
    EXPECT_CALL(*mockColumn_5, value(An<const std::string&>())).WillOnce(Return(""));

    EXPECT_CALL(*mockColumn_1, hasValue()).WillOnce(Return(true));
    EXPECT_CALL(*mockColumn_2, hasValue()).WillOnce(Return(true));
    EXPECT_CALL(*mockColumn_3, hasValue()).WillOnce(Return(true));
    EXPECT_CALL(*mockColumn_4, hasValue()).WillOnce(Return(true));
    EXPECT_CALL(*mockColumn_5, hasValue()).WillOnce(Return(true));

    EXPECT_CALL(*mockStatement, column(0)).WillOnce(Return(ByMove(std::move(mockColumn_1))));
    EXPECT_CALL(*mockStatement, column(1)).WillOnce(Return(ByMove(std::move(mockColumn_2))));
    EXPECT_CALL(*mockStatement, column(2)).WillOnce(Return(ByMove(std::move(mockColumn_3))));
    EXPECT_CALL(*mockStatement, column(3)).WillOnce(Return(ByMove(std::move(mockColumn_4))));
    EXPECT_CALL(*mockStatement, column(4)).WillOnce(Return(ByMove(std::move(mockColumn_5))));

    MacportsWrapper macportsMock(*mockStatement);

    nlohmann::json package;
    macportsMock.name(package);
    macportsMock.version(package);
    macportsMock.install_time(package);
    macportsMock.location(package);
    macportsMock.architecture(package);

    EXPECT_EQ(package["name"], "neovim");
    // Empty string fields are replaced with space.
    EXPECT_EQ(package["version"], EMPTY_VALUE);
    EXPECT_FALSE(package["install_time"].empty());
    EXPECT_EQ(package["location"], EMPTY_VALUE);
    EXPECT_EQ(package["architecture"], EMPTY_VALUE);
}

TEST_F(SysInfoMacPackagesTest, macPortsValidDataEmptyName)
{
    auto mockStatement {std::make_unique<MockStatement>()};
    EXPECT_CALL(*mockStatement, columnsCount()).WillOnce(Return(5));

    auto mockColumn_1 {std::make_unique<MockColumn>()};
    EXPECT_CALL(*mockColumn_1, value(An<const std::string&>())).WillOnce(Return(""));
    auto mockColumn_2 {std::make_unique<MockColumn>()};
    EXPECT_CALL(*mockColumn_2, value(An<const std::string&>())).WillOnce(Return("0.8.1"));
    auto mockColumn_3 {std::make_unique<MockColumn>()};
    EXPECT_CALL(*mockColumn_3, value(An<const int64_t&>())).WillOnce(Return(1690831043));
    auto mockColumn_4 {std::make_unique<MockColumn>()};
    EXPECT_CALL(*mockColumn_4, value(An<const std::string&>()))
        .WillOnce(Return("/opt/local/var/macports/software/neovim/neovim-0.8.1.tgz"));
    auto mockColumn_5 {std::make_unique<MockColumn>()};
    EXPECT_CALL(*mockColumn_5, value(An<const std::string&>())).WillOnce(Return("x86_64"));

    EXPECT_CALL(*mockColumn_1, hasValue()).WillOnce(Return(true));
    EXPECT_CALL(*mockColumn_2, hasValue()).WillOnce(Return(true));
    EXPECT_CALL(*mockColumn_3, hasValue()).WillOnce(Return(true));
    EXPECT_CALL(*mockColumn_4, hasValue()).WillOnce(Return(true));
    EXPECT_CALL(*mockColumn_5, hasValue()).WillOnce(Return(true));

    EXPECT_CALL(*mockStatement, column(0)).WillOnce(Return(ByMove(std::move(mockColumn_1))));
    EXPECT_CALL(*mockStatement, column(1)).WillOnce(Return(ByMove(std::move(mockColumn_2))));
    EXPECT_CALL(*mockStatement, column(2)).WillOnce(Return(ByMove(std::move(mockColumn_3))));
    EXPECT_CALL(*mockStatement, column(3)).WillOnce(Return(ByMove(std::move(mockColumn_4))));
    EXPECT_CALL(*mockStatement, column(4)).WillOnce(Return(ByMove(std::move(mockColumn_5))));

    MacportsWrapper macportsMock(*mockStatement);

    nlohmann::json package;
    macportsMock.name(package);

    // Packages with empty string names are discarded.
    EXPECT_EQ(package["name"], "");
}
