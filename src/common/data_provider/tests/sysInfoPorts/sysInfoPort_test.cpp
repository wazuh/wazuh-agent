#include "sysInfoPort_test.hpp"
#include "ports/portImpl.h"

void SysInfoPortTest::SetUp() {};

void SysInfoPortTest::TearDown() {};

using ::testing::_;
using ::testing::Return;

class SysInfoPortWrapperMock : public IPortWrapper
{
public:
    SysInfoPortWrapperMock() = default;
    virtual ~SysInfoPortWrapperMock() = default;
    MOCK_METHOD(void, protocol, (nlohmann::json&), (const override));
    MOCK_METHOD(void, localIp, (nlohmann::json&), (const override));
    MOCK_METHOD(void, localPort, (nlohmann::json&), (const override));
    MOCK_METHOD(void, remoteIP, (nlohmann::json&), (const override));
    MOCK_METHOD(void, remotePort, (nlohmann::json&), (const override));
    MOCK_METHOD(void, txQueue, (nlohmann::json&), (const override));
    MOCK_METHOD(void, rxQueue, (nlohmann::json&), (const override));
    MOCK_METHOD(void, inode, (nlohmann::json&), (const override));
    MOCK_METHOD(void, state, (nlohmann::json&), (const override));
    MOCK_METHOD(void, pid, (nlohmann::json&), (const override));
    MOCK_METHOD(void, processName, (nlohmann::json&), (const override));
};

TEST_F(SysInfoPortTest, Test_SPEC_Data)
{
    auto mock {std::make_shared<SysInfoPortWrapperMock>()};
    nlohmann::json port {};
    EXPECT_CALL(*mock, protocol(_)).Times(1).WillOnce([](nlohmann::json& p) { p["protocol"] = "1"; });
    EXPECT_CALL(*mock, localIp(_)).Times(1).WillOnce([](nlohmann::json& p) { p["local_ip"] = "2"; });
    EXPECT_CALL(*mock, localPort(_)).Times(1).WillOnce([](nlohmann::json& p) { p["local_port"] = 3; });
    EXPECT_CALL(*mock, remoteIP(_)).Times(1).WillOnce([](nlohmann::json& p) { p["remote_ip"] = "4"; });
    EXPECT_CALL(*mock, remotePort(_)).Times(1).WillOnce([](nlohmann::json& p) { p["remote_port"] = 5; });
    EXPECT_CALL(*mock, txQueue(_)).Times(1).WillOnce([](nlohmann::json& p) { p["tx_queue"] = 6; });
    EXPECT_CALL(*mock, rxQueue(_)).Times(1).WillOnce([](nlohmann::json& p) { p["rx_queue"] = 7; });
    EXPECT_CALL(*mock, inode(_)).Times(1).WillOnce([](nlohmann::json& p) { p["inode"] = 4274126910; });
    EXPECT_CALL(*mock, state(_)).Times(1).WillOnce([](nlohmann::json& p) { p["state"] = "9"; });
    EXPECT_CALL(*mock, pid(_)).Times(1).WillOnce([](nlohmann::json& p) { p["pid"] = 10; });
    EXPECT_CALL(*mock, processName(_)).Times(1).WillOnce([](nlohmann::json& p) { p["process"] = "11"; });

    EXPECT_NO_THROW(std::make_unique<PortImpl>(mock)->buildPortData(port));
    EXPECT_EQ("1", port.at("protocol").get_ref<const std::string&>());
    EXPECT_EQ("2", port.at("local_ip").get_ref<const std::string&>());
    EXPECT_EQ(3, port.at("local_port").get<int32_t>());
    EXPECT_EQ("4", port.at("remote_ip").get_ref<const std::string&>());
    EXPECT_EQ(5, port.at("remote_port").get<int32_t>());
    EXPECT_EQ(6, port.at("tx_queue").get<int32_t>());
    EXPECT_EQ(7, port.at("rx_queue").get<int32_t>());
    EXPECT_EQ(4274126910, port.at("inode").get<int64_t>());
    EXPECT_EQ("9", port.at("state").get_ref<const std::string&>());
    EXPECT_EQ(10, port.at("pid").get<int32_t>());
    EXPECT_EQ("11", port.at("process").get_ref<const std::string&>());
}
