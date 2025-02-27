#include "../http_client/tests/mocks/mock_http_client.hpp"
#include <agent.hpp>
#include <cstdio>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <isignal_handler.hpp>
#include <mock_agent_info.hpp>
#include <mock_command_store.hpp>
#include <mock_multitype_queue.hpp>

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
};

TEST_F(AgentTests, AgentStopsWhenSignalReceived)
{
    auto mockSignalHandler = std::make_unique<MockSignalHandler>();
    MockSignalHandler* mockSignalHandlerPtr = mockSignalHandler.get();

    auto mockHttpClient = std::make_unique<MockHttpClient>();

    auto mockCommandStore = std::make_unique<command_store::MockCommandStore>();

    auto mockMultiTypeQueue = std::make_shared<MockMultiTypeQueue>();

    auto mockAgentInfo = std::make_unique<MockAgentInfo>();
    MockAgentInfo* mockAgentInfoPtr = mockAgentInfo.get();

    auto WaitForSignalCalled = false;

    const std::vector<std::string> mockGroups = {"group1", "group2"};
    EXPECT_CALL(*mockAgentInfoPtr, GetUUID()).Times(3).WillRepeatedly(testing::Return("mock_uuid"));
    EXPECT_CALL(*mockAgentInfoPtr, GetKey()).Times(2).WillRepeatedly(testing::Return("mock_key"));
    EXPECT_CALL(*mockAgentInfoPtr, GetName()).WillOnce(testing::Return("mock_name"));
    EXPECT_CALL(*mockAgentInfoPtr, GetGroups()).WillOnce(testing::Return(mockGroups));
    EXPECT_CALL(*mockAgentInfoPtr, GetHeaderInfo()).WillRepeatedly(testing::Return("header_info"));

    EXPECT_CALL(*mockSignalHandlerPtr, WaitForSignal())
        .Times(1)
        .WillOnce([&WaitForSignalCalled]() { WaitForSignalCalled = true; });

    intStringTuple expectedResponse {http_client::HTTP_CODE_UNAUTHORIZED, R"({"message":"Try again"})"};

    EXPECT_CALL(*mockHttpClient, PerformHttpRequest(testing::_))
        .WillRepeatedly(testing::Invoke([&expectedResponse]() -> intStringTuple { return expectedResponse; }));

    EXPECT_CALL(*mockCommandStore, GetCommandByStatus(testing::_)).WillOnce(testing::Return(std::nullopt));

    Agent agent(AGENT_CONFIG_PATH,
                std::move(mockSignalHandler),
                std::move(mockHttpClient),
                std::move(mockAgentInfo),
                std::move(mockCommandStore),
                std::move(mockMultiTypeQueue));

    EXPECT_NO_THROW(agent.Run());
    EXPECT_TRUE(WaitForSignalCalled);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
