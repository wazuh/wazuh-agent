#include <agent.hpp>
#include <boost/asio/awaitable.hpp>
#include <cstdio>
#include <fstream>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <isignal_handler.hpp>
#include <mock_agent_info.hpp>
#include <mock_command_handler.hpp>
#include <mock_http_client.hpp>
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

    const std::string m_configString = R"(
        agent:
          server_url: https://localhost:27000
          path.data: )" + AGENT_PATH + R"(
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
};

TEST_F(AgentTests, AgentStopsWhenSignalReceived)
{
    auto mockSignalHandler = std::make_unique<MockSignalHandler>();
    MockSignalHandler* mockSignalHandlerPtr = mockSignalHandler.get();

    auto mockHttpClient = std::make_unique<MockHttpClient>();

    auto mockCommandHandler = std::make_unique<command_handler::MockCommandHandler>();
    command_handler::MockCommandHandler* mockCommandHandlerPtr = mockCommandHandler.get();

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

    EXPECT_CALL(*mockCommandHandlerPtr, CommandsProcessingTask(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Invoke([](auto, auto, auto, auto) -> boost::asio::awaitable<void> { co_return; }));

    EXPECT_CALL(*mockCommandHandlerPtr, Stop()).Times(1);

    Agent agent(std::make_unique<configuration::ConfigurationParser>(m_configString),
                std::move(mockSignalHandler),
                std::move(mockHttpClient),
                std::move(mockAgentInfo),
                std::move(mockCommandHandler),
                std::move(mockMultiTypeQueue));

    EXPECT_NO_THROW(agent.Run());
    EXPECT_TRUE(WaitForSignalCalled);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
