#include <message_queue_utils.hpp>

#include <message.hpp>
#include <multitype_queue.hpp>

#include <boost/asio.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

const nlohmann::json BASE_DATA_CONTENT = R"([{"data": {"id":"112233", "args": ["origin_test",
                                        "command_test", "parameters_test"]}}])"_json;

class MockMultiTypeQueue : public MultiTypeQueue
{
public:
    MOCK_METHOD(boost::asio::awaitable<Message>,
                getNextNAwaitable,
                (MessageType, int, const std::string module),
                (override));
    MOCK_METHOD(int, popN, (MessageType, int, const std::string module), (override));
    MOCK_METHOD(int, push, (Message, bool), (override));
    MOCK_METHOD(int, push, (std::vector<Message>), (override));
    MOCK_METHOD(bool, isEmpty, (MessageType, const std::string moduleName), (override));
    MOCK_METHOD(Message, getNext, (MessageType, const std::string moduleName), (override));
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
    std::vector<std::string> data {"test_data"};
    Message testMessage {MessageType::STATEFUL, data};

    // NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    EXPECT_CALL(*mockQueue, getNextNAwaitable(MessageType::STATEFUL, 1, ""))
        .WillOnce([&testMessage]() -> boost::asio::awaitable<Message> { co_return testMessage; });
    // NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)

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

TEST_F(MessageQueueUtilsTest, GetCommandFromQueueEmptyTest)
{
    EXPECT_CALL(*mockQueue, isEmpty(MessageType::COMMAND, "")).WillOnce(testing::Return(true));

    ASSERT_EQ(std::nullopt, GetCommandFromQueue(mockQueue));
}

TEST_F(MessageQueueUtilsTest, GetCommandFromQueueTest)
{
    Message testMessage {MessageType::COMMAND, BASE_DATA_CONTENT};

    EXPECT_CALL(*mockQueue, isEmpty(MessageType::COMMAND, "")).WillOnce(testing::Return(false));

    EXPECT_CALL(*mockQueue, getNext(MessageType::COMMAND, "")).WillOnce(testing::Return(testMessage));

    auto cmd = GetCommandFromQueue(mockQueue);

    ASSERT_EQ(cmd.has_value() ? cmd.value().Id : "", "112233");
    ASSERT_EQ(cmd.has_value() ? cmd.value().Module : "", "origin_test");
    ASSERT_EQ(cmd.has_value() ? cmd.value().Command : "", "command_test");
    ASSERT_EQ(cmd.has_value() ? cmd.value().Parameters : "", "parameters_test");
    ASSERT_EQ(cmd.has_value() ? cmd.value().ExecutionResult.ErrorCode : module_command::Status::UNKNOWN,
              module_command::Status::IN_PROGRESS);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
