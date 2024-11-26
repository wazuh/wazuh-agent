#include <agent.hpp>
#include <cstdio>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <isignal_handler.hpp>

constexpr auto AGENT_CONFIG_PATH {"/tmp/wazuh-agent.yml"};

class MockSignalHandler : public ISignalHandler
{
public:
    MOCK_METHOD(void, WaitForSignal, (), (override));
};

class AgentTests : public ::testing::Test
{
protected:
    void SetUp() override
    {
        CreateTempConfigFile();
    }

    void TearDown() override
    {
        std::remove(AGENT_CONFIG_PATH);
    }

    void CreateTempConfigFile()
    {
        std::ofstream configFile(AGENT_CONFIG_PATH);
        configFile << R"(
agent:
  server_url: https://localhost:27000
  path.data: /tmp/
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
  file_wait: 500ms
)";
        configFile.close();
    }
};

TEST_F(AgentTests, AgentDefaultConstruction)
{
    EXPECT_NO_THROW(Agent {AGENT_CONFIG_PATH});
}

TEST_F(AgentTests, AgentStopsWhenSignalReceived)
{
    auto mockSignalHandler = std::make_unique<MockSignalHandler>();
    MockSignalHandler* mockSignalHandlerPtr = mockSignalHandler.get();

    EXPECT_CALL(*mockSignalHandlerPtr, WaitForSignal())
        .Times(1)
        .WillOnce([]() { std::this_thread::sleep_for(std::chrono::seconds(1)); });

    Agent agent(AGENT_CONFIG_PATH, std::move(mockSignalHandler));

    EXPECT_NO_THROW(agent.Run());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
