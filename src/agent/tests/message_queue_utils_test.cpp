#include <message_queue_utils.hpp>

#include <message.hpp>
#include <multitype_queue.hpp>

#include <boost/asio.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

class MockMultiTypeQueue : public MultiTypeQueue
{
public:
    MOCK_METHOD(boost::asio::awaitable<Message>,
                getNextNAwaitable,
                (MessageType, int, const std::string module),
                (override));
    MOCK_METHOD(int, popN, (MessageType, int, const std::string module), (override));
    MOCK_METHOD(int, push, (std::vector<Message>), (override));
};

class MessageQueueUtilsTest : public ::testing::Test
{
protected:
    MessageQueueUtilsTest()
        : mockQueue(std::make_shared<MockMultiTypeQueue>())
    {
    }

    boost::asio::io_context io_context;
    std::shared_ptr<MockMultiTypeQueue> mockQueue;
};

TEST_F(MessageQueueUtilsTest, GetMessagesFromQueueTest)
{
    Message testMessage {MessageType::STATEFUL, "test_data"};

    EXPECT_CALL(*mockQueue, getNextNAwaitable(MessageType::STATEFUL, 1, ""))
        .WillOnce([&testMessage]() -> boost::asio::awaitable<Message> { co_return testMessage; });

    io_context.restart();

    auto result = boost::asio::co_spawn(
        io_context, GetMessagesFromQueue(mockQueue, MessageType::STATEFUL), boost::asio::use_future);

    const auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(1);
    io_context.run_until(timeout);

    ASSERT_TRUE(result.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready);

    const auto jsonResult = result.get();

    nlohmann::json expectedJson;
    expectedJson["events"] = nlohmann::json::array();
    expectedJson["events"].push_back("test_data");

    ASSERT_EQ(jsonResult, expectedJson.dump());
}

TEST_F(MessageQueueUtilsTest, PopMessagesFromQueueTest)
{
    EXPECT_CALL(*mockQueue, popN(MessageType::STATEFUL, 1, "")).Times(1);
    PopMessagesFromQueue(mockQueue, MessageType::STATEFUL);
}

TEST_F(MessageQueueUtilsTest, PushCommandsToQueueTest)
{
    nlohmann::json commandsJson;
    commandsJson["commands"] = nlohmann::json::array();
    commandsJson["commands"].push_back("command_1");
    commandsJson["commands"].push_back("command_2");

    std::vector<Message> expectedMessages;
    expectedMessages.emplace_back(MessageType::COMMAND, "command_1");
    expectedMessages.emplace_back(MessageType::COMMAND, "command_2");

    EXPECT_CALL(*mockQueue, push(::testing::ContainerEq(expectedMessages))).Times(1);

    PushCommandsToQueue(mockQueue, commandsJson.dump());
}

TEST_F(MessageQueueUtilsTest, NoCommandsToPushTest)
{
    nlohmann::json commandsJson;
    commandsJson["commands"] = nlohmann::json::array();

    EXPECT_CALL(*mockQueue, push(::testing::_)).Times(0);

    PushCommandsToQueue(mockQueue, commandsJson.dump());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
