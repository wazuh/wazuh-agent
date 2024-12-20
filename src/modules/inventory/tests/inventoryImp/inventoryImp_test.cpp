#include <cstdio>
#include <gtest/gtest.h>
#include "inventoryImp_test.hpp"
#include "inventory.hpp"

constexpr auto INVENTORY_DB_PATH {"TEMP.db"};
constexpr int SLEEP_DURATION_SECONDS = 3;

void ReportFunction(const std::string& payload);

void InventoryImpTest::SetUp() {};

void InventoryImpTest::TearDown()
{
    std::remove(INVENTORY_DB_PATH);
};

using ::testing::Return;

class SysInfoWrapper: public ISysInfo
{
    public:
        SysInfoWrapper() = default;
        ~SysInfoWrapper() override = default;

        SysInfoWrapper(const SysInfoWrapper&) = delete;
        SysInfoWrapper& operator=(const SysInfoWrapper&) = delete;
        SysInfoWrapper(SysInfoWrapper&&) = delete;
        SysInfoWrapper& operator=(SysInfoWrapper&&) = delete;

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

        CallbackMock(const CallbackMock&) = delete;
        CallbackMock& operator=(const CallbackMock&) = delete;
        CallbackMock(CallbackMock&&) = delete;
        CallbackMock& operator=(CallbackMock&&) = delete;

        MOCK_METHOD(void, callbackMock, (const std::string&), ());
};

void ReportFunction(const std::string& /*payload*/)
{
    // std::cout << payload << std::endl;
}

TEST_F(InventoryImpTest, defaultCtor)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})"
    )));

    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})"
    )));
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                   R"([{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0}])")));
    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));

    EXPECT_CALL(*spInfoWrapper, processes(testing::_))
    .Times(testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","scan_time":"2020/12/28 21:49:50", "nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0})"_json));

    EXPECT_CALL(*spInfoWrapper, hotfixes()).WillRepeatedly(Return(nlohmann::json::parse(R"([{"hotfix":"KB12345678"}])")));

    EXPECT_CALL(*spInfoWrapper, networks()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"iface":[{"IPv4":[{"address":"172.17.0.1","broadcast":"172.17.255.255","dhcp":"unknown","metric":"0","netmask":"255.255.0.0"}],"adapter":"","gateway":" ","mac":"02:42:1c:26:13:65","mtu":1500,"name":"docker0","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"state":"down","tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0,"type":"ethernet"}]})")));

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("@timestamp");
            delta.erase("id");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"host":{"cpu":{"cores":2,"name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","speed":null},"memory":{"free":2257872,"total":4972208,"used":{"percentage":54}}},"observer":{"serial_number":"Intel Corporation"}},"operation":"create","type":"hardware"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"host":{"architecture":"x86_64","hostname":"UBUNTU","os":{"full":null,"kernel":"7601","name":"Microsoft Windows 7","platform":null,"type":null,"version":"6.1.7601"}}},"operation":"create","type":"system"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"package":{"architecture":"amd64","description":null,"installed":null,"name":"xserver-xorg","path":" ","size":411,"type":"deb","version":"1:7.7+19ubuntu14"}},"operation":"create","type":"packages"})"
    };
    const auto expectedResult4
    {
        R"({"data":{"process":{"args":null,"command_line":null,"group":{"id":"root"},"name":"kworker/u256:2-","parent":{"pid":2},"pid":"431625","real_group":{"id":"root"},"real_user":{"id":"root"},"saved_group":{"id":"root"},"saved_user":{"id":"root"},"start":9302261,"thread":{"id":431625},"tty":{"char_device":{"major":0}},"user":{"id":"root"}}},"operation":"create","type":"processes"})"
    };
    const auto expectedResult5
    {
        R"({"data":{"package":{"hotfix":{"name":"KB12345678"}}},"operation":"create","type":"hotfixes"})"
    };
    const auto expectedResult6
    {
        R"({"data":{"destination":{"ip":["0.0.0.0"],"port":0},"device":{"id":"cbf2ac25a6775175f912ebf2abc72f6f51ab48ba"},"file":{"inode":0},"host":{"network":{"egress":{"queue":0},"ingress":{"queue":0}}},"interface":{"state":"listening"},"network":{"protocol":"tcp"},"process":{"name":null,"pid":0},"source":{"ip":["127.0.0.1"],"port":631}},"operation":"create","type":"ports"})"
    };
    const auto expectedResult7
    {
        R"({"data":{"host":{"ip":["172.17.0.1"],"mac":"02:42:1c:26:13:65","network":{"egress":{"bytes":0,"drops":0,"errors":0,"packets":0},"ingress":{"bytes":0,"drops":0,"errors":0,"packets":0}}},"interface":{"mtu":1500,"state":"down","type":"ethernet"},"network":{"broadcast":["172.17.255.255"],"dhcp":"unknown","gateway":[],"metric":"0","netmask":["255.255.0.0"],"protocol":null,"type":"ipv4"},"observer":{"ingress":{"interface":{"alias":null,"name":"docker0"}}}},"operation":"create","type":"networks"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(testing::AtLeast(1));
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(testing::AtLeast(1));
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(testing::AtLeast(1));
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult4)).Times(testing::AtLeast(1));
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult5)).Times(testing::AtLeast(1));
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult6)).Times(testing::AtLeast(1));
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult7)).Times(testing::AtLeast(1));

    auto configurationParser = std::make_shared<configuration::ConfigurationParser>();
    Inventory::Instance().Setup(configurationParser);
    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                        callbackDataDelta,
                                        INVENTORY_DB_PATH,
                                        "",
                                        "");
            Inventory::Instance().SetAgentUUID("1234");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds(1));
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, intervalSeconds)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})"
    )));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})"
    )));
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                   R"([{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0}])")));
    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(2))
    .WillRepeatedly(::testing::InvokeArgument<0>
                    (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));
    EXPECT_CALL(*spInfoWrapper, processes(testing::_))
    .Times(testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","scan_time":"2020/12/28 21:49:50", "nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0})"_json));

    EXPECT_CALL(*spInfoWrapper, hotfixes()).WillRepeatedly(Return(R"([{"hotfix":"KB12345678"},{"hotfix":"KB87654321"}])"_json));

    EXPECT_CALL(*spInfoWrapper, networks()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"iface":[{"IPv4":[{"address":"172.17.0.1","broadcast":"172.17.255.255","dhcp":"unknown","metric":"0","netmask":"255.255.0.0"}],"adapter":"","gateway":"","mac":"02:42:1c:26:13:65","mtu":1500,"name":"docker0","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"state":"down","tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0,"type":"ethernet"}]})")));

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 1
            scan_on_start: true
            hardware: true
            system: true
            networks: true
            packages: true
            ports: true
            ports_all: true
            processes: true
            hotfixes: true
    )";
    auto configParser = std::make_shared<configuration::ConfigurationParser>(inventoryConfig);
    Inventory::Instance().Setup(configParser);

    std::thread t
    {
        [&spInfoWrapper]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          ReportFunction,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
            Inventory::Instance().SetAgentUUID("1234");

        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{SLEEP_DURATION_SECONDS});
    Inventory::Instance().Stop();

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
    EXPECT_CALL(*spInfoWrapper, packages(testing::_)).Times(0);
    EXPECT_CALL(*spInfoWrapper, networks()).Times(0);
    EXPECT_CALL(*spInfoWrapper, processes(testing::_)).Times(0);
    EXPECT_CALL(*spInfoWrapper, ports()).Times(0);
    EXPECT_CALL(*spInfoWrapper, hotfixes()).Times(0);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: false
            hardware: true
            system: true
            networks: true
            packages: true
            ports: true
            ports_all: true
            processes: true
            hotfixes: true
    )";
    auto configParser = std::make_shared<configuration::ConfigurationParser>(inventoryConfig);
    Inventory::Instance().Setup(configParser);

    std::thread t
    {
        [&spInfoWrapper]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                        ReportFunction,
                                        INVENTORY_DB_PATH,
                                        "",
                                        "");
            Inventory::Instance().SetAgentUUID("1234");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noHardware)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})"
    )));
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                   R"([{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0}])")));
    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));
    EXPECT_CALL(*spInfoWrapper, processes(testing::_))
    .Times(testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","scan_time":"2020/12/28 21:49:50", "nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0})"_json));
    EXPECT_CALL(*spInfoWrapper, hotfixes()).WillRepeatedly(Return(nlohmann::json::parse(R"([{"hotfix":"KB12345678"}])")));
    EXPECT_CALL(*spInfoWrapper, networks()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"iface":[{"IPv4":[{"address":"172.17.0.1","broadcast":"172.17.255.255","dhcp":"unknown","metric":"0","netmask":"255.255.0.0"}],"adapter":"","gateway":"","mac":"02:42:1c:26:13:65","mtu":1500,"name":"docker0","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"state":"down","tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0,"type":"ethernet"}]})")));

    EXPECT_CALL(*spInfoWrapper, hardware()).Times(0);

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("@timestamp");
            delta.erase("id");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult2
    {
        R"({"data":{"host":{"architecture":"x86_64","hostname":"UBUNTU","os":{"full":"","kernel":"7601","name":"Microsoft Windows 7","platform":"","type":"","version":"6.1.7601"}}},"operation":"create","type":"system"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"package":{"architecture":"amd64","description":"","installed":null,"name":"xserver-xorg","path":" ","size":411,"type":"deb","version":"1:7.7+19ubuntu14"}},"operation":"create","type":"packages"})"
    };
    const auto expectedResult4
    {
        R"({"data":{"process":{"args":"","command_line":"","group":{"id":"root"},"name":"kworker/u256:2-","parent":{"pid":2},"pid":"431625","real_group":{"id":"root"},"real_user":{"id":"root"},"saved_group":{"id":"root"},"saved_user":{"id":"root"},"start":9302261,"thread":{"id":431625},"tty":{"char_device":{"major":0}},"user":{"id":"root"}}},"operation":"create","type":"processes"})"
    };
    const auto expectedResult5
    {
        R"({"data":{"package":{"hotfix":{"name":"KB12345678"}}},"operation":"create","type":"hotfixes"})"
    };
    const auto expectedResult6
    {
        R"({"data":{"destination":{"ip":["0.0.0.0"],"port":0},"device":{"id":"cbf2ac25a6775175f912ebf2abc72f6f51ab48ba"},"file":{"inode":0},"host":{"network":{"egress":{"queue":0},"ingress":{"queue":0}}},"interface":{"state":"listening"},"network":{"protocol":"tcp"},"process":{"name":"","pid":0},"source":{"ip":["127.0.0.1"],"port":631}},"operation":"create","type":"ports"})"
    };
    const auto expectedResult7
    {
        R"({"data":{"host":{"ip":["172.17.0.1"],"mac":"02:42:1c:26:13:65","network":{"egress":{"bytes":0,"drops":0,"errors":0,"packets":0},"ingress":{"bytes":0,"drops":0,"errors":0,"packets":0}}},"interface":{"mtu":1500,"state":"down","type":"ethernet"},"network":{"broadcast":["172.17.255.255"],"dhcp":"unknown","gateway":[],"metric":"0","netmask":["255.255.0.0"],"protocol":"","type":"ipv4"},"observer":{"ingress":{"interface":{"alias":"","name":"docker0"}}}},"operation":"create","type":"networks"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult4)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult5)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult6)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult7)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: false
            system: true
            networks: true
            packages: true
            ports: true
            ports_all: true
            processes: true
            hotfixes: true
    )";
    auto configParser = std::make_shared<configuration::ConfigurationParser>(inventoryConfig);
    Inventory::Instance().Setup(configParser);

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                        callbackDataDelta,
                                        INVENTORY_DB_PATH,
                                        "",
                                        "");
            Inventory::Instance().SetAgentUUID("1234");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds(1));
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }

}

TEST_F(InventoryImpTest, noOs)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})"
    )));
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                   R"([{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0}])")));
    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));
    EXPECT_CALL(*spInfoWrapper, processes(testing::_))
    .Times(testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","scan_time":"2020/12/28 21:49:50", "nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0})"_json));
    EXPECT_CALL(*spInfoWrapper, hotfixes()).WillRepeatedly(Return(nlohmann::json::parse(R"([{"hotfix":"KB12345678"}])")));
    EXPECT_CALL(*spInfoWrapper, networks()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"iface":[{"IPv4":[{"address":"172.17.0.1","broadcast":"172.17.255.255","dhcp":"unknown","metric":"0","netmask":"255.255.0.0"}],"adapter":"","gateway":"","mac":"02:42:1c:26:13:65","mtu":1500,"name":"docker0","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"state":"down","tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0,"type":"ethernet"}]})")));

    EXPECT_CALL(*spInfoWrapper, os()).Times(0);

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("@timestamp");
            delta.erase("id");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"host":{"cpu":{"cores":2,"name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","speed":null},"memory":{"free":2257872,"total":4972208,"used":{"percentage":54}}},"observer":{"serial_number":"Intel Corporation"}},"operation":"create","type":"hardware"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"package":{"architecture":"amd64","description":"","installed":null,"name":"xserver-xorg","path":" ","size":411,"type":"deb","version":"1:7.7+19ubuntu14"}},"operation":"create","type":"packages"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"process":{"args":"","command_line":"","group":{"id":"root"},"name":"kworker/u256:2-","parent":{"pid":2},"pid":"431625","real_group":{"id":"root"},"real_user":{"id":"root"},"saved_group":{"id":"root"},"saved_user":{"id":"root"},"start":9302261,"thread":{"id":431625},"tty":{"char_device":{"major":0}},"user":{"id":"root"}}},"operation":"create","type":"processes"})"
    };
    const auto expectedResult4
    {
        R"({"data":{"package":{"hotfix":{"name":"KB12345678"}}},"operation":"create","type":"hotfixes"})"
    };
    const auto expectedResult5
    {
        R"({"data":{"destination":{"ip":["0.0.0.0"],"port":0},"device":{"id":"cbf2ac25a6775175f912ebf2abc72f6f51ab48ba"},"file":{"inode":0},"host":{"network":{"egress":{"queue":0},"ingress":{"queue":0}}},"interface":{"state":"listening"},"network":{"protocol":"tcp"},"process":{"name":"","pid":0},"source":{"ip":["127.0.0.1"],"port":631}},"operation":"create","type":"ports"})"
    };
    const auto expectedResult7
    {
        R"({"data":{"host":{"ip":["172.17.0.1"],"mac":"02:42:1c:26:13:65","network":{"egress":{"bytes":0,"drops":0,"errors":0,"packets":0},"ingress":{"bytes":0,"drops":0,"errors":0,"packets":0}}},"interface":{"mtu":1500,"state":"down","type":"ethernet"},"network":{"broadcast":["172.17.255.255"],"dhcp":"unknown","gateway":[],"metric":"0","netmask":["255.255.0.0"],"protocol":"","type":"ipv4"},"observer":{"ingress":{"interface":{"alias":"","name":"docker0"}}}},"operation":"create","type":"networks"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult4)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult5)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult7)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: true
            system: false
            networks: true
            packages: true
            ports: true
            ports_all: true
            processes: true
            hotfixes: true
    )";
    auto configParser = std::make_shared<configuration::ConfigurationParser>(inventoryConfig);
    Inventory::Instance().Setup(configParser);

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          callbackDataDelta,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
            Inventory::Instance().SetAgentUUID("1234");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds(2));
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noNetwork)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})"
    )));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})"
    )));
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                   R"([{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0}])")));
    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));
    EXPECT_CALL(*spInfoWrapper, processes(testing::_))
    .Times(testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","scan_time":"2020/12/28 21:49:50", "nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0})"_json));
    EXPECT_CALL(*spInfoWrapper, hotfixes()).WillRepeatedly(Return(nlohmann::json::parse(R"([{"hotfix":"KB12345678"}])")));

    EXPECT_CALL(*spInfoWrapper, networks()).Times(0);

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("@timestamp");
            delta.erase("id");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"host":{"cpu":{"cores":2,"name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","speed":null},"memory":{"free":2257872,"total":4972208,"used":{"percentage":54}}},"observer":{"serial_number":"Intel Corporation"}},"operation":"create","type":"hardware"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"host":{"architecture":"x86_64","hostname":"UBUNTU","os":{"full":null,"kernel":"7601","name":"Microsoft Windows 7","platform":null,"type":null,"version":"6.1.7601"}}},"operation":"create","type":"system"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"package":{"architecture":"amd64","description":null,"installed":null,"name":"xserver-xorg","path":" ","size":411,"type":"deb","version":"1:7.7+19ubuntu14"}},"operation":"create","type":"packages"})"
    };
    const auto expectedResult4
    {
        R"({"data":{"process":{"args":null,"command_line":null,"group":{"id":"root"},"name":"kworker/u256:2-","parent":{"pid":2},"pid":"431625","real_group":{"id":"root"},"real_user":{"id":"root"},"saved_group":{"id":"root"},"saved_user":{"id":"root"},"start":9302261,"thread":{"id":431625},"tty":{"char_device":{"major":0}},"user":{"id":"root"}}},"operation":"create","type":"processes"})"
    };
    const auto expectedResult5
    {
        R"({"data":{"package":{"hotfix":{"name":"KB12345678"}}},"operation":"create","type":"hotfixes"})"
    };
    const auto expectedResult6
    {
        R"({"data":{"destination":{"ip":["0.0.0.0"],"port":0},"device":{"id":"cbf2ac25a6775175f912ebf2abc72f6f51ab48ba"},"file":{"inode":0},"host":{"network":{"egress":{"queue":0},"ingress":{"queue":0}}},"interface":{"state":"listening"},"network":{"protocol":"tcp"},"process":{"name":null,"pid":0},"source":{"ip":["127.0.0.1"],"port":631}},"operation":"create","type":"ports"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult4)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult5)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult6)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: true
            system: true
            networks: false
            packages: true
            ports: true
            ports_all: true
            processes: true
            hotfixes: true
    )";
    auto configParser = std::make_shared<configuration::ConfigurationParser>(inventoryConfig);
    Inventory::Instance().Setup(configParser);

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                        callbackDataDelta,
                                        INVENTORY_DB_PATH,
                                        "",
                                        "");
            Inventory::Instance().SetAgentUUID("1234");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds(1));
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noPackages)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})"
    )));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})"
    )));
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                   R"([{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0}])")));
    EXPECT_CALL(*spInfoWrapper, processes(testing::_))
    .Times(testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","scan_time":"2020/12/28 21:49:50", "nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0})"_json));
    EXPECT_CALL(*spInfoWrapper, hotfixes()).WillRepeatedly(Return(nlohmann::json::parse(R"([{"hotfix":"KB12345678"}])")));
    EXPECT_CALL(*spInfoWrapper, networks()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"iface":[{"IPv4":[{"address":"172.17.0.1","broadcast":"172.17.255.255","dhcp":"unknown","metric":"0","netmask":"255.255.0.0"}],"adapter":"","gateway":"","mac":"02:42:1c:26:13:65","mtu":1500,"name":"docker0","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"state":"down","tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0,"type":"ethernet"}]})")));

    EXPECT_CALL(*spInfoWrapper, packages()).Times(0);


    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("@timestamp");
            delta.erase("id");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"host":{"cpu":{"cores":2,"name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","speed":null},"memory":{"free":2257872,"total":4972208,"used":{"percentage":54}}},"observer":{"serial_number":"Intel Corporation"}},"operation":"create","type":"hardware"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"host":{"architecture":"x86_64","hostname":"UBUNTU","os":{"full":"","kernel":"7601","name":"Microsoft Windows 7","platform":"","type":"","version":"6.1.7601"}}},"operation":"create","type":"system"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"process":{"args":"","command_line":"","group":{"id":"root"},"name":"kworker/u256:2-","parent":{"pid":2},"pid":"431625","real_group":{"id":"root"},"real_user":{"id":"root"},"saved_group":{"id":"root"},"saved_user":{"id":"root"},"start":9302261,"thread":{"id":431625},"tty":{"char_device":{"major":0}},"user":{"id":"root"}}},"operation":"create","type":"processes"})"
    };
    const auto expectedResult4
    {
        R"({"data":{"package":{"hotfix":{"name":"KB12345678"}}},"operation":"create","type":"hotfixes"})"
    };
    const auto expectedResult5
    {
        R"({"data":{"destination":{"ip":["0.0.0.0"],"port":0},"device":{"id":"cbf2ac25a6775175f912ebf2abc72f6f51ab48ba"},"file":{"inode":0},"host":{"network":{"egress":{"queue":0},"ingress":{"queue":0}}},"interface":{"state":"listening"},"network":{"protocol":"tcp"},"process":{"name":"","pid":0},"source":{"ip":["127.0.0.1"],"port":631}},"operation":"create","type":"ports"})"
    };
    const auto expectedResult7
    {
        R"({"data":{"host":{"ip":["172.17.0.1"],"mac":"02:42:1c:26:13:65","network":{"egress":{"bytes":0,"drops":0,"errors":0,"packets":0},"ingress":{"bytes":0,"drops":0,"errors":0,"packets":0}}},"interface":{"mtu":1500,"state":"down","type":"ethernet"},"network":{"broadcast":["172.17.255.255"],"dhcp":"unknown","gateway":[],"metric":"0","netmask":["255.255.0.0"],"protocol":"","type":"ipv4"},"observer":{"ingress":{"interface":{"alias":"","name":"docker0"}}}},"operation":"create","type":"networks"})"
    };


    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult4)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult5)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult7)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: true
            system: true
            networks: true
            packages: false
            ports: true
            ports_all: true
            processes: true
            hotfixes: true
    )";
    auto configParser = std::make_shared<configuration::ConfigurationParser>(inventoryConfig);
    Inventory::Instance().Setup(configParser);

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          callbackDataDelta,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
            Inventory::Instance().SetAgentUUID("1234");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noPorts)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})"
    )));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})"
    )));
    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));
    EXPECT_CALL(*spInfoWrapper, processes(testing::_))
    .Times(testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","scan_time":"2020/12/28 21:49:50", "nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0})"_json));
    EXPECT_CALL(*spInfoWrapper, hotfixes()).WillRepeatedly(Return(nlohmann::json::parse(R"([{"hotfix":"KB12345678"}])")));
    EXPECT_CALL(*spInfoWrapper, networks()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"iface":[{"IPv4":[{"address":"172.17.0.1","broadcast":"172.17.255.255","dhcp":"unknown","metric":"0","netmask":"255.255.0.0"}],"adapter":"","gateway":"","mac":"02:42:1c:26:13:65","mtu":1500,"name":"docker0","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"state":"down","tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0,"type":"ethernet"}]})")));

    EXPECT_CALL(*spInfoWrapper, ports()).Times(0);

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("@timestamp");
            delta.erase("id");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"host":{"cpu":{"cores":2,"name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","speed":null},"memory":{"free":2257872,"total":4972208,"used":{"percentage":54}}},"observer":{"serial_number":"Intel Corporation"}},"operation":"create","type":"hardware"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"host":{"architecture":"x86_64","hostname":"UBUNTU","os":{"full":"","kernel":"7601","name":"Microsoft Windows 7","platform":"","type":"","version":"6.1.7601"}}},"operation":"create","type":"system"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"package":{"architecture":"amd64","description":"","installed":null,"name":"xserver-xorg","path":" ","size":411,"type":"deb","version":"1:7.7+19ubuntu14"}},"operation":"create","type":"packages"})"
    };
    const auto expectedResult4
    {
        R"({"data":{"process":{"args":"","command_line":"","group":{"id":"root"},"name":"kworker/u256:2-","parent":{"pid":2},"pid":"431625","real_group":{"id":"root"},"real_user":{"id":"root"},"saved_group":{"id":"root"},"saved_user":{"id":"root"},"start":9302261,"thread":{"id":431625},"tty":{"char_device":{"major":0}},"user":{"id":"root"}}},"operation":"create","type":"processes"})"
    };
    const auto expectedResult5
    {
        R"({"data":{"package":{"hotfix":{"name":"KB12345678"}}},"operation":"create","type":"hotfixes"})"
    };
    const auto expectedResult7
    {
        R"({"data":{"host":{"ip":["172.17.0.1"],"mac":"02:42:1c:26:13:65","network":{"egress":{"bytes":0,"drops":0,"errors":0,"packets":0},"ingress":{"bytes":0,"drops":0,"errors":0,"packets":0}}},"interface":{"mtu":1500,"state":"down","type":"ethernet"},"network":{"broadcast":["172.17.255.255"],"dhcp":"unknown","gateway":[],"metric":"0","netmask":["255.255.0.0"],"protocol":"","type":"ipv4"},"observer":{"ingress":{"interface":{"alias":"","name":"docker0"}}}},"operation":"create","type":"networks"})"
    };


    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult4)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult5)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult7)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 5
            scan_on_start: true
            hardware: true
            system: true
            networks: true
            packages: true
            ports: false
            ports_all: true
            processes: true
            hotfixes: true
    )";
    auto configParser = std::make_shared<configuration::ConfigurationParser>(inventoryConfig);
    Inventory::Instance().Setup(configParser);

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          callbackDataDelta,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
            Inventory::Instance().SetAgentUUID("1234");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds(1));
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noPortsAll)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})"
    )));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})"
    )));
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                   R"([{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"udp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"","tx_queue":0},{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0}])")));
    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));
    EXPECT_CALL(*spInfoWrapper, processes(testing::_))
    .Times(testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","scan_time":"2020/12/28 21:49:50", "nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0})"_json));
    EXPECT_CALL(*spInfoWrapper, hotfixes()).WillRepeatedly(Return(nlohmann::json::parse(R"([{"hotfix":"KB12345678"}])")));

    EXPECT_CALL(*spInfoWrapper, networks()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"iface":[{"IPv4":[{"address":"172.17.0.1","broadcast":"172.17.255.255","dhcp":"unknown","metric":"0","netmask":"255.255.0.0"}],"adapter":"","gateway":"","mac":"02:42:1c:26:13:65","mtu":1500,"name":"docker0","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"state":"down","tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0,"type":"ethernet"}]})")));

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("@timestamp");
            delta.erase("id");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"host":{"cpu":{"cores":2,"name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","speed":null},"memory":{"free":2257872,"total":4972208,"used":{"percentage":54}}},"observer":{"serial_number":"Intel Corporation"}},"operation":"create","type":"hardware"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"host":{"architecture":"x86_64","hostname":"UBUNTU","os":{"full":"","kernel":"7601","name":"Microsoft Windows 7","platform":"","type":"","version":"6.1.7601"}}},"operation":"create","type":"system"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"package":{"architecture":"amd64","description":"","installed":null,"name":"xserver-xorg","path":" ","size":411,"type":"deb","version":"1:7.7+19ubuntu14"}},"operation":"create","type":"packages"})"
    };
    const auto expectedResult4
    {
        R"({"data":{"process":{"args":"","command_line":"","group":{"id":"root"},"name":"kworker/u256:2-","parent":{"pid":2},"pid":"431625","real_group":{"id":"root"},"real_user":{"id":"root"},"saved_group":{"id":"root"},"saved_user":{"id":"root"},"start":9302261,"thread":{"id":431625},"tty":{"char_device":{"major":0}},"user":{"id":"root"}}},"operation":"create","type":"processes"})"
    };
    const auto expectedResult5
    {
        R"({"data":{"package":{"hotfix":{"name":"KB12345678"}}},"operation":"create","type":"hotfixes"})"
    };
    const auto expectedResult6
    {
        R"({"data":{"destination":{"ip":["0.0.0.0"],"port":0},"device":{"id":"7046b3f9cda975eb6567259c2469748e634dde49"},"file":{"inode":0},"host":{"network":{"egress":{"queue":0},"ingress":{"queue":0}}},"interface":{"state":""},"network":{"protocol":"udp"},"process":{"name":"","pid":0},"source":{"ip":["127.0.0.1"],"port":631}},"operation":"create","type":"ports"})"
    };
    const auto expectedResult7
    {
        R"({"data":{"destination":{"ip":["0.0.0.0"],"port":0},"device":{"id":"cbf2ac25a6775175f912ebf2abc72f6f51ab48ba"},"file":{"inode":0},"host":{"network":{"egress":{"queue":0},"ingress":{"queue":0}}},"interface":{"state":"listening"},"network":{"protocol":"tcp"},"process":{"name":"","pid":0},"source":{"ip":["127.0.0.1"],"port":631}},"operation":"create","type":"ports"})"
    };
    const auto expectedResult8
    {
        R"({"data":{"host":{"ip":["172.17.0.1"],"mac":"02:42:1c:26:13:65","network":{"egress":{"bytes":0,"drops":0,"errors":0,"packets":0},"ingress":{"bytes":0,"drops":0,"errors":0,"packets":0}}},"interface":{"mtu":1500,"state":"down","type":"ethernet"},"network":{"broadcast":["172.17.255.255"],"dhcp":"unknown","gateway":[],"metric":"0","netmask":["255.255.0.0"],"protocol":"","type":"ipv4"},"observer":{"ingress":{"interface":{"alias":"","name":"docker0"}}}},"operation":"create","type":"networks"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult4)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult5)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult6)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult7)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult8)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: true
            system: true
            networks: true
            packages: true
            ports: true
            ports_all: false
            processes: true
            hotfixes: true
    )";
    auto configParser = std::make_shared<configuration::ConfigurationParser>(inventoryConfig);
    Inventory::Instance().Setup(configParser);

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          callbackDataDelta,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
            Inventory::Instance().SetAgentUUID("1234");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds(1));
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noProcesses)
{

    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})"
    )));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})"
    )));
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                   R"([{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0}])")));
    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));
    EXPECT_CALL(*spInfoWrapper, hotfixes()).WillRepeatedly(Return(nlohmann::json::parse(R"([{"hotfix":"KB12345678"}])")));
    EXPECT_CALL(*spInfoWrapper, networks()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"iface":[{"IPv4":[{"address":"172.17.0.1","broadcast":"172.17.255.255","dhcp":"unknown","metric":"0","netmask":"255.255.0.0"}],"adapter":"","gateway":"","mac":"02:42:1c:26:13:65","mtu":1500,"name":"docker0","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"state":"down","tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0,"type":"ethernet"}]})")));

    EXPECT_CALL(*spInfoWrapper, processes(testing::_)).Times(0);

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("@timestamp");
            delta.erase("id");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"host":{"cpu":{"cores":2,"name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","speed":null},"memory":{"free":2257872,"total":4972208,"used":{"percentage":54}}},"observer":{"serial_number":"Intel Corporation"}},"operation":"create","type":"hardware"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"host":{"architecture":"x86_64","hostname":"UBUNTU","os":{"full":"","kernel":"7601","name":"Microsoft Windows 7","platform":"","type":"","version":"6.1.7601"}}},"operation":"create","type":"system"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"package":{"architecture":"amd64","description":"","installed":null,"name":"xserver-xorg","path":" ","size":411,"type":"deb","version":"1:7.7+19ubuntu14"}},"operation":"create","type":"packages"})"
    };
    const auto expectedResult4
    {
        R"({"data":{"package":{"hotfix":{"name":"KB12345678"}}},"operation":"create","type":"hotfixes"})"
    };
    const auto expectedResult5
    {
        R"({"data":{"destination":{"ip":["0.0.0.0"],"port":0},"device":{"id":"cbf2ac25a6775175f912ebf2abc72f6f51ab48ba"},"file":{"inode":0},"host":{"network":{"egress":{"queue":0},"ingress":{"queue":0}}},"interface":{"state":"listening"},"network":{"protocol":"tcp"},"process":{"name":"","pid":0},"source":{"ip":["127.0.0.1"],"port":631}},"operation":"create","type":"ports"})"
    };
    const auto expectedResult7
    {
        R"({"data":{"host":{"ip":["172.17.0.1"],"mac":"02:42:1c:26:13:65","network":{"egress":{"bytes":0,"drops":0,"errors":0,"packets":0},"ingress":{"bytes":0,"drops":0,"errors":0,"packets":0}}},"interface":{"mtu":1500,"state":"down","type":"ethernet"},"network":{"broadcast":["172.17.255.255"],"dhcp":"unknown","gateway":[],"metric":"0","netmask":["255.255.0.0"],"protocol":"","type":"ipv4"},"observer":{"ingress":{"interface":{"alias":"","name":"docker0"}}}},"operation":"create","type":"networks"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult4)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult5)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult7)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: true
            system: true
            networks: true
            packages: true
            ports: true
            ports_all: true
            processes: false
            hotfixes: true
    )";
    auto configParser = std::make_shared<configuration::ConfigurationParser>(inventoryConfig);
    Inventory::Instance().Setup(configParser);

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          callbackDataDelta,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
            Inventory::Instance().SetAgentUUID("1234");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, noHotfixes)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, hardware()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"board_serial":"Intel Corporation","scan_time":"2020/12/28 21:49:50", "cpu_MHz":2904,"cpu_cores":2,"cpu_name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz", "ram_free":2257872,"ram_total":4972208,"ram_usage":54})"
    )));
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})"
    )));
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                   R"([{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0}])")));
    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"architecture":"amd64","scan_time":"2020/12/28 21:49:50", "group":"x11","name":"xserver-xorg","priority":"optional","size":411,"source":"xorg","version":"1:7.7+19ubuntu14","format":"deb","location":" "})"_json));
    EXPECT_CALL(*spInfoWrapper, processes(testing::_))
    .Times(testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","scan_time":"2020/12/28 21:49:50", "nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0})"_json));
    EXPECT_CALL(*spInfoWrapper, networks()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"iface":[{"IPv4":[{"address":"172.17.0.1","broadcast":"172.17.255.255","dhcp":"unknown","metric":"0","netmask":"255.255.0.0"}],"adapter":"","gateway":"","mac":"02:42:1c:26:13:65","mtu":1500,"name":"docker0","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"state":"down","tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0,"type":"ethernet"}]})")));

    EXPECT_CALL(*spInfoWrapper, hotfixes()).Times(0);

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("@timestamp");
            delta.erase("id");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"host":{"cpu":{"cores":2,"name":"Intel(R) Core(TM) i5-9400 CPU @ 2.90GHz","speed":null},"memory":{"free":2257872,"total":4972208,"used":{"percentage":54}}},"observer":{"serial_number":"Intel Corporation"}},"operation":"create","type":"hardware"})"
    };
    const auto expectedResult2
    {
        R"({"data":{"host":{"architecture":"x86_64","hostname":"UBUNTU","os":{"full":"","kernel":"7601","name":"Microsoft Windows 7","platform":"","type":"","version":"6.1.7601"}}},"operation":"create","type":"system"})"
    };
    const auto expectedResult3
    {
        R"({"data":{"package":{"architecture":"amd64","description":"","installed":null,"name":"xserver-xorg","path":" ","size":411,"type":"deb","version":"1:7.7+19ubuntu14"}},"operation":"create","type":"packages"})"
    };
    const auto expectedResult4
    {
        R"({"data":{"process":{"args":"","command_line":"","group":{"id":"root"},"name":"kworker/u256:2-","parent":{"pid":2},"pid":"431625","real_group":{"id":"root"},"real_user":{"id":"root"},"saved_group":{"id":"root"},"saved_user":{"id":"root"},"start":9302261,"thread":{"id":431625},"tty":{"char_device":{"major":0}},"user":{"id":"root"}}},"operation":"create","type":"processes"})"
    };
    const auto expectedResult5
    {
        R"({"data":{"destination":{"ip":["0.0.0.0"],"port":0},"device":{"id":"cbf2ac25a6775175f912ebf2abc72f6f51ab48ba"},"file":{"inode":0},"host":{"network":{"egress":{"queue":0},"ingress":{"queue":0}}},"interface":{"state":"listening"},"network":{"protocol":"tcp"},"process":{"name":"","pid":0},"source":{"ip":["127.0.0.1"],"port":631}},"operation":"create","type":"ports"})"
    };
    const auto expectedResult7
    {
        R"({"data":{"host":{"ip":["172.17.0.1"],"mac":"02:42:1c:26:13:65","network":{"egress":{"bytes":0,"drops":0,"errors":0,"packets":0},"ingress":{"bytes":0,"drops":0,"errors":0,"packets":0}}},"interface":{"mtu":1500,"state":"down","type":"ethernet"},"network":{"broadcast":["172.17.255.255"],"dhcp":"unknown","gateway":[],"metric":"0","netmask":["255.255.0.0"],"protocol":"","type":"ipv4"},"observer":{"ingress":{"interface":{"alias":"","name":"docker0"}}}},"operation":"create","type":"networks"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult3)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult4)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult5)).Times(1);
    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult7)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: true
            system: true
            networks: true
            packages: true
            ports: true
            ports_all: true
            processes: true
            hotfixes: false
    )";
    auto configParser = std::make_shared<configuration::ConfigurationParser>(inventoryConfig);
    Inventory::Instance().Setup(configParser);

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          callbackDataDelta,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
            Inventory::Instance().SetAgentUUID("1234");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::Instance().Stop();

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
    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})")));
    EXPECT_CALL(*spInfoWrapper, ports()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                   R"([{"inode":0,"local_ip":"127.0.0.1","scan_time":"2020/12/28 21:49:50", "local_port":631,"pid":0,"process_name":"System Idle Process","protocol":"tcp","remote_ip":"0.0.0.0","remote_port":0,"rx_queue":0,"state":"listening","tx_queue":0}])")));
    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
    .Times(::testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"name":"TEXT", "scan_time":"2020/12/28 21:49:50", "version":"TEXT", "vendor":"TEXT", "install_time":"TEXT", "location":"TEXT", "architecture":"TEXT", "groups":"TEXT", "description":"TEXT", "size":"TEXT", "priority":"TEXT", "multiarch":"TEXT", "source":"TEXT", "os_patch":"TEXT"})"_json));
    EXPECT_CALL(*spInfoWrapper, processes(testing::_))
    .Times(testing::AtLeast(1))
    .WillOnce(::testing::InvokeArgument<0>
              (R"({"egroup":"root","euser":"root","fgroup":"root","name":"kworker/u256:2-","scan_time":"2020/12/28 21:49:50", "nice":0,"nlwp":1,"pgrp":0,"pid":"431625","ppid":2,"priority":20,"processor":1,"resident":0,"rgroup":"root","ruser":"root","session":0,"sgroup":"root","share":0,"size":0,"start_time":9302261,"state":"I","stime":3,"suser":"root","tgid":431625,"tty":0,"utime":0,"vm_size":0})"_json));
    EXPECT_CALL(*spInfoWrapper, hotfixes()).WillRepeatedly(Return(nlohmann::json::parse(R"([{"hotfix":"KB12345678"}])")));
    EXPECT_CALL(*spInfoWrapper, networks()).WillRepeatedly(Return(nlohmann::json::parse(
                                                                      R"({"iface":[{"IPv4":[{"address":"172.17.0.1","broadcast":"172.17.255.255","dhcp":"unknown","metric":"0","netmask":"255.255.0.0"}],"adapter":"","gateway":"","mac":"02:42:1c:26:13:65","mtu":1500,"name":"docker0","rx_bytes":0,"rx_dropped":0,"rx_errors":0,"rx_packets":0,"state":"down","tx_bytes":0,"tx_dropped":0,"tx_errors":0,"tx_packets":0,"type":"ethernet"}]})")));

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 60
            scan_on_start: true
            hardware: true
            system: true
            networks: true
            packages: true
            ports: true
            ports_all: true
            processes: true
            hotfixes: true
    )";
    auto configParser = std::make_shared<configuration::ConfigurationParser>(inventoryConfig);
    Inventory::Instance().Setup(configParser);

    std::thread t
    {
        [&spInfoWrapper]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          ReportFunction,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
            Inventory::Instance().SetAgentUUID("1234");
        }
    };
    std::this_thread::sleep_for(std::chrono::seconds{1});
    Inventory::Instance().Stop();

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
            delta["data"].erase("@timestamp");
            delta.erase("id");
            wrapper.callbackMock(delta.dump());
        }
    };
    const auto expectedResult1
    {
        R"({"data":{"destination":{"ip":["0.0.0.0"],"port":0},"device":{"id":"12903a43db24ab10d872547cdd1d786a5876a0da"},"file":{"inode":43481},"host":{"network":{"egress":{"queue":0},"ingress":{"queue":0}}},"interface":{"state":""},"network":{"protocol":"udp"},"process":{"name":"","pid":0},"source":{"ip":["0.0.0.0"],"port":47748}},"operation":"create","type":"ports"})"
    };

    const auto expectedResult2
    {
        R"({"data":{"destination":{"ip":["::"],"port":0},"device":{"id":"ca7c9aff241cb251c6ad31e30b806366ecb2ad5f"},"file":{"inode":43482},"host":{"network":{"egress":{"queue":0},"ingress":{"queue":0}}},"interface":{"state":""},"network":{"protocol":"udp6"},"process":{"name":"","pid":0},"source":{"ip":["::"],"port":51087}},"operation":"create","type":"ports"})"
    };

    const auto expectedResult3
    {
        R"({"data":{"destination":{"ip":["0.0.0.0"],"port":0},"device":{"id":"8c790ef53962dd27f4516adb1d7f3f6096bc6d29"},"file":{"inode":50324},"host":{"network":{"egress":{"queue":0},"ingress":{"queue":0}}},"interface":{"state":"listening"},"network":{"protocol":"tcp"},"process":{"name":"","pid":0},"source":{"ip":["127.0.0.1"],"port":33060}},"operation":"create","type":"ports"})"
    };

    const auto expectedResult4
    {
        R"({"data":{"destination":{"ip":["44.238.116.130"],"port":443},"device":{"id":"d5511242275bd3f2d57175f248108d6c3b39c438"},"file":{"inode":122575},"host":{"network":{"egress":{"queue":0},"ingress":{"queue":0}}},"interface":{"state":"established"},"network":{"protocol":"tcp"},"process":{"name":"","pid":0},"source":{"ip":["192.168.0.104"],"port":39106}},"operation":"create","type":"ports"})"
    };

    EXPECT_CALL(wrapper, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapper, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapper, callbackMock(expectedResult3)).Times(1);
    EXPECT_CALL(wrapper, callbackMock(expectedResult4)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: false
            system: false
            networks: false
            packages: false
            ports: true
            ports_all: true
            processes: false
            hotfixes: false
    )";
    auto configParser = std::make_shared<configuration::ConfigurationParser>(inventoryConfig);
    Inventory::Instance().Setup(configParser);

    std::thread t
    {
        [&spInfoWrapper, &callbackData]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          callbackData,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
            Inventory::Instance().SetAgentUUID("1234");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::Instance().Stop();

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
            delta["data"].erase("@timestamp");
            delta.erase("id");
            delta.erase("id");
            wrapper.callbackMock(delta.dump());
        }
    };
    const auto expectedResult1
    {
        R"({"data":{"destination":{"ip":["0.0.0.0"],"port":0},"device":{"id":"12903a43db24ab10d872547cdd1d786a5876a0da"},"file":{"inode":43481},"host":{"network":{"egress":{"queue":0},"ingress":{"queue":0}}},"interface":{"state":""},"network":{"protocol":"udp"},"process":{"name":"","pid":0},"source":{"ip":["0.0.0.0"],"port":47748}},"operation":"create","type":"ports"})"
    };

    const auto expectedResult2
    {
        R"({"data":{"destination":{"ip":["::"],"port":0},"device":{"id":"ca7c9aff241cb251c6ad31e30b806366ecb2ad5f"},"file":{"inode":43482},"host":{"network":{"egress":{"queue":0},"ingress":{"queue":0}}},"interface":{"state":""},"network":{"protocol":"udp6"},"process":{"name":"","pid":0},"source":{"ip":["::"],"port":51087}},"operation":"create","type":"ports"})"
    };

    const auto expectedResult3
    {
        R"({"data":{"destination":{"ip":["0.0.0.0"],"port":0},"device":{"id":"8c790ef53962dd27f4516adb1d7f3f6096bc6d29"},"file":{"inode":50324},"host":{"network":{"egress":{"queue":0},"ingress":{"queue":0}}},"interface":{"state":"listening"},"network":{"protocol":"tcp"},"process":{"name":"","pid":0},"source":{"ip":["127.0.0.1"],"port":33060}},"operation":"create","type":"ports"})"
    };

    EXPECT_CALL(wrapper, callbackMock(expectedResult1)).Times(1);
    EXPECT_CALL(wrapper, callbackMock(expectedResult2)).Times(1);
    EXPECT_CALL(wrapper, callbackMock(expectedResult3)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: false
            system: false
            networks: false
            packages: false
            ports: true
            ports_all: false
            processes: false
            hotfixes: false
    )";
    auto configParser = std::make_shared<configuration::ConfigurationParser>(inventoryConfig);
    Inventory::Instance().Setup(configParser);

    std::thread t
    {
        [&spInfoWrapper, &callbackData]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          callbackData,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
            Inventory::Instance().SetAgentUUID("1234");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, PackagesDuplicated)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, packages(testing::_))
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
            delta["data"].erase("@timestamp");
            delta.erase("id");
            wrapper.callbackMock(delta.dump());
        }
    };

    const auto expectedResult1
    {
        R"({"data":{"package":{"architecture":"amd64","description":"","installed":null,"name":"xserver-xorg","path":" ","size":411,"type":"deb","version":"1:7.7+19ubuntu14"}},"operation":"create","type":"packages"})"
    };

    EXPECT_CALL(wrapper, callbackMock(expectedResult1)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: false
            system: false
            networks: false
            packages: true
            ports: false
            ports_all: false
            processes: false
            hotfixes: false
    )";
    auto configParser = std::make_shared<configuration::ConfigurationParser>(inventoryConfig);
    Inventory::Instance().Setup(configParser);

    std::thread t
    {
        [&spInfoWrapper, &callbackData]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                          callbackData,
                                          INVENTORY_DB_PATH,
                                          "",
                                          "");
            Inventory::Instance().SetAgentUUID("1234");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds{2});
    Inventory::Instance().Stop();

    if (t.joinable())
    {
        t.join();
    }
}

TEST_F(InventoryImpTest, hashId)
{
    const auto spInfoWrapper{std::make_shared<SysInfoWrapper>()};

    EXPECT_CALL(*spInfoWrapper, os()).WillRepeatedly(Return(nlohmann::json::parse(
        R"({"architecture":"x86_64","scan_time":"2020/12/28 21:49:50", "hostname":"UBUNTU","os_build":"7601","os_major":"6","os_minor":"1","os_name":"Microsoft Windows 7","os_release":"sp1","os_version":"6.1.7601"})"
    )));

    CallbackMock wrapperDelta;
    std::function<void(const std::string&)> callbackDataDelta
    {
        [&wrapperDelta](const std::string & data)
        {
            auto delta = nlohmann::json::parse(data);
            delta["data"].erase("@timestamp");
            wrapperDelta.callbackMock(delta.dump());
        }
    };

    const auto expectedResult2
    {
        R"({"data":{"host":{"architecture":"x86_64","hostname":"UBUNTU","os":{"full":"","kernel":"7601","name":"Microsoft Windows 7","platform":"","type":"","version":"6.1.7601"}}},"id":"6bd3291be0d2314de0329e8ac36be434a085eb32","operation":"create","type":"system"})"
    };

    EXPECT_CALL(wrapperDelta, callbackMock(expectedResult2)).Times(1);

    std::string inventoryConfig = R"(
        inventory:
            enabled: true
            interval: 3600
            scan_on_start: true
            hardware: false
            system: true
            networks: false
            packages: false
            ports: false
            ports_all: false
            processes: false
            hotfixes: false
    )";
    auto configParser = std::make_shared<configuration::ConfigurationParser>(inventoryConfig);
    Inventory::Instance().Setup(configParser);

    std::thread t
    {
        [&spInfoWrapper, &callbackDataDelta]()
        {
            Inventory::Instance().Init(spInfoWrapper,
                                        callbackDataDelta,
                                        INVENTORY_DB_PATH,
                                        "",
                                        "");
            Inventory::Instance().SetAgentUUID("1234");
        }
    };

    std::this_thread::sleep_for(std::chrono::seconds(1));
    Inventory::Instance().Stop();

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
