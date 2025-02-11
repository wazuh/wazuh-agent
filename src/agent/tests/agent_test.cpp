#include "../http_client/tests/mocks/mock_http_client.hpp"
#include <agent.hpp>
#include <agent_info_persistance.hpp>
#include <cstdio>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <isignal_handler.hpp>
#include <mocks_persistence.hpp>

class MockSignalHandler : public ISignalHandler
{
public:
    MOCK_METHOD(void, WaitForSignal, (), (override));
};

class AgentTests : public ::testing::Test
{
protected:
    std::string AGENT_CONFIG_PATH;
    std::string AGENT_PATH;
    std::unique_ptr<AgentInfo> m_agentInfo;
    MockPersistence* m_mockPersistence = nullptr;

    void SetUp() override
    {
#ifdef WIN32
        char* tmpPath = nullptr;
        size_t len = 0;

        _dupenv_s(&tmpPath, &len, "TMP");
        std::string tempFolder = std::string(tmpPath);
        AGENT_CONFIG_PATH = tempFolder + "wazuh-agent.yml";
        AGENT_PATH = tempFolder;
#else
        AGENT_CONFIG_PATH = "/tmp/wazuh-agent.yml";
        AGENT_PATH = "/tmp";
#endif

        CreateTempConfigFile();

        auto mockPersistencePtr = std::make_unique<MockPersistence>();
        m_mockPersistence = mockPersistencePtr.get();

        SetConstructorPersistenceExpectCalls();

        SysInfo sysInfo;
        m_agentInfo = std::make_unique<AgentInfo>(
            AGENT_PATH,
            [sysInfo]() mutable { return sysInfo.os(); },
            [sysInfo]() mutable { return sysInfo.networks(); },
            false,
            std::make_shared<AgentInfoPersistance>("db_path", std::move(mockPersistencePtr)));

        m_agentInfo->SetKey("4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj");
        m_agentInfo->SetName("agent_name");
    }

    void TearDown() override
    {
        CleanUpTempFiles();
    }

    void CreateTempConfigFile()
    {
        CleanUpTempFiles();

        std::ofstream configFilePath(AGENT_CONFIG_PATH);
        configFilePath << R"(
agent:
  server_url: https://localhost:27000
  path.data: )" << AGENT_PATH
                       << R"(
  retry_interval: 30s
inventory:
  enabled: false
  interval: 1h
  scan_on_start: true
  hardware: true
  os: true
  network: true
  packages: true
  ports: true
  ports_all: true
  processes: true
  hotfixes: true
logcollector:
  enabled: false
  localfiles:
    - /var/log/auth.log
  reload_interval: 1m
  read_interval: 500ms
)";
        configFilePath.close();
    }

    void CleanUpTempFiles()
    {
        std::remove(AGENT_CONFIG_PATH.c_str());
    }

    void SetConstructorPersistenceExpectCalls()
    {
        std::vector<column::Row> mockRowName = {{column::ColumnValue("name", column::ColumnType::TEXT, "agent_name")}};
        std::vector<column::Row> mockRowKey = {
            {column::ColumnValue("key", column::ColumnType::TEXT, "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj")}};
        std::vector<column::Row> mockRowUUID = {{}};
        std::vector<column::Row> mockRowGroup = {{}};

        EXPECT_CALL(*m_mockPersistence, TableExists("agent_info")).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockPersistence, TableExists("agent_group")).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockPersistence, GetCount("agent_info", testing::_, testing::_))
            .WillOnce(testing::Return(0))
            .WillOnce(testing::Return(0));
        EXPECT_CALL(*m_mockPersistence, Insert("agent_info", testing::_)).Times(1);

        testing::Sequence seq;
        EXPECT_CALL(*m_mockPersistence,
                    Select("agent_info", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
            .InSequence(seq)
            .WillOnce(testing::Return(mockRowName))
            .WillOnce(testing::Return(mockRowKey))
            .WillOnce(testing::Return(mockRowUUID));
        EXPECT_CALL(*m_mockPersistence,
                    Select("agent_group", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
            .WillOnce(testing::Return(mockRowGroup));
    }
};

TEST_F(AgentTests, AgentStopsWhenSignalReceived)
{
    auto mockSignalHandler = std::make_unique<MockSignalHandler>();
    MockSignalHandler* mockSignalHandlerPtr = mockSignalHandler.get();

    auto mockHttpClient = std::make_unique<MockHttpClient>();

    auto WaitForSignalCalled = false;

    EXPECT_CALL(*mockSignalHandlerPtr, WaitForSignal())
        .Times(1)
        .WillOnce([&WaitForSignalCalled]() { WaitForSignalCalled = true; });

    intStringTuple expectedResponse {http_client::HTTP_CODE_UNAUTHORIZED, R"({"message":"Try again"})"};

    EXPECT_CALL(*mockHttpClient, PerformHttpRequest(testing::_))
        .WillRepeatedly(testing::Invoke([&expectedResponse]() -> intStringTuple { return expectedResponse; }));

    Agent agent(AGENT_CONFIG_PATH, std::move(mockSignalHandler), std::move(mockHttpClient), std::move(*m_agentInfo));

    EXPECT_NO_THROW(agent.Run());
    EXPECT_TRUE(WaitForSignalCalled);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
