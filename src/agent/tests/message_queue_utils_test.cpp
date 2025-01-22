#include <message_queue_utils.hpp>

#include <message.hpp>
#include <multitype_queue.hpp>

#include <boost/asio.hpp>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <nlohmann/json.hpp>

const nlohmann::json BASE_DATA_CONTENT =
    R"({"document_id":"112233", "action":{"name":"command_test","args":{"parameters":["parameters_test"]}}})"_json;

const auto FUNC = []<typename T>([[maybe_unused]] const std::string&,
                                 [[maybe_unused]] const std::string&) -> std::optional<T>
{
    return T {};
};

class MockMultiTypeQueue : public MultiTypeQueue
{
public:
    MockMultiTypeQueue()
        : MultiTypeQueue(FUNC)
    {
    }

    MOCK_METHOD(boost::asio::awaitable<std::vector<Message>>,
                getNextBytesAwaitable,
                (MessageType, const size_t, const std::string, const std::string),
                (override));
    MOCK_METHOD(int, popN, (MessageType, int, const std::string, const std::string), (override));
    MOCK_METHOD(int, push, (Message, bool), (override));
    MOCK_METHOD(int, push, (std::vector<Message>), (override));
    MOCK_METHOD(bool, isEmpty, (MessageType, const std::string, const std::string), (override));
    MOCK_METHOD(Message, getNext, (MessageType, const std::string, const std::string), (override));
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

    const size_t MIN_SIZE_OF_MESSAGES = 10;
};

TEST_F(MessageQueueUtilsTest, GetMessagesFromQueueTestBySize)
{
    std::vector<std::string> data {R"({"event":{"original":"Testing message!"}})"};
    std::string metadata {R"({"module":"logcollector","type":"file"})"};
    std::vector<Message> testMessages;
    testMessages.emplace_back(MessageType::STATELESS, data, "", "", metadata);

    // NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    EXPECT_CALL(*mockQueue, getNextBytesAwaitable(MessageType::STATELESS, MIN_SIZE_OF_MESSAGES, "", ""))
        .WillOnce([&testMessages]() -> boost::asio::awaitable<std::vector<Message>> { co_return testMessages; });
    // NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)

    auto awaitableResult =
        boost::asio::co_spawn(io_context,
                              GetMessagesFromQueue(mockQueue, MessageType::STATELESS, MIN_SIZE_OF_MESSAGES, nullptr),
                              boost::asio::use_future);

    const auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(1);
    io_context.run_until(timeout);

    ASSERT_TRUE(awaitableResult.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready);

    const auto result = awaitableResult.get();
    const auto jsonResult = std::get<1>(result);

    std::string expectedString = std::string("\n") + R"({"module":"logcollector","type":"file"})" + std::string("\n") +
                                 R"(["{\"event\":{\"original\":\"Testing message!\"}}"])";

    ASSERT_EQ(jsonResult, expectedString);
}

TEST_F(MessageQueueUtilsTest, GetMessagesFromQueueMetadataTest)
{
    std::vector<std::string> data {R"({"event":{"original":"Testing message!"}})"};
    std::string moduleMetadata {R"({"module":"logcollector","type":"file"})"};
    std::vector<Message> testMessages;
    testMessages.emplace_back(MessageType::STATELESS, data, "", "", moduleMetadata);

    nlohmann::json metadata;
    metadata["agent"] = "test";

    // NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    EXPECT_CALL(*mockQueue, getNextBytesAwaitable(MessageType::STATELESS, MIN_SIZE_OF_MESSAGES, "", ""))
        .WillOnce([&testMessages]() -> boost::asio::awaitable<std::vector<Message>> { co_return testMessages; });
    // NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)

    io_context.restart();

    auto awaitableResult = boost::asio::co_spawn(
        io_context,
        GetMessagesFromQueue(
            mockQueue, MessageType::STATELESS, MIN_SIZE_OF_MESSAGES, [&metadata]() { return metadata.dump(); }),
        boost::asio::use_future);

    const auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(1);
    io_context.run_until(timeout);

    ASSERT_TRUE(awaitableResult.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready);

    const auto result = awaitableResult.get();
    const auto jsonResult = std::get<1>(result);

    std::string expectedString = R"({"agent":"test"})" + std::string("\n") +
                                 R"({"module":"logcollector","type":"file"})" + std::string("\n") +
                                 R"(["{\"event\":{\"original\":\"Testing message!\"}}"])";

    ASSERT_EQ(jsonResult, expectedString);
}

TEST_F(MessageQueueUtilsTest, GetEmptyMessagesFromQueueTest)
{
    nlohmann::json data = nlohmann::json::object();
    std::string moduleMetadata {R"({"operation":"delete"})"};
    std::vector<Message> testMessages;
    testMessages.emplace_back(MessageType::STATEFUL, data, "", "", moduleMetadata);

    nlohmann::json metadata;
    metadata["agent"] = "test";

    // NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    EXPECT_CALL(*mockQueue, getNextBytesAwaitable(MessageType::STATEFUL, MIN_SIZE_OF_MESSAGES, "", ""))
        .WillOnce([&testMessages]() -> boost::asio::awaitable<std::vector<Message>> { co_return testMessages; });
    // NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)

    io_context.restart();

    auto awaitableResult = boost::asio::co_spawn(
        io_context,
        GetMessagesFromQueue(
            mockQueue, MessageType::STATEFUL, MIN_SIZE_OF_MESSAGES, [&metadata]() { return metadata.dump(); }),
        boost::asio::use_future);

    const auto timeout = std::chrono::steady_clock::now() + std::chrono::milliseconds(1);
    io_context.run_until(timeout);

    ASSERT_TRUE(awaitableResult.wait_for(std::chrono::milliseconds(1)) == std::future_status::ready);

    const auto result = awaitableResult.get();
    const auto jsonResult = std::get<1>(result);

    std::string expectedString = R"({"agent":"test"})" + std::string("\n") + R"({"operation":"delete"})";

    ASSERT_EQ(jsonResult, expectedString);
}

TEST_F(MessageQueueUtilsTest, PopMessagesFromQueueTest)
{
    EXPECT_CALL(*mockQueue, popN(MessageType::STATEFUL, 1, "", "")).Times(1);
    PopMessagesFromQueue(mockQueue, MessageType::STATEFUL, 1);
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
    EXPECT_CALL(*mockQueue, isEmpty(MessageType::COMMAND, "", "")).WillOnce(testing::Return(true));

    ASSERT_EQ(std::nullopt, GetCommandFromQueue(mockQueue));
}

TEST_F(MessageQueueUtilsTest, GetCommandFromQueueTest)
{
    Message testMessage {MessageType::COMMAND, BASE_DATA_CONTENT};

    EXPECT_CALL(*mockQueue, isEmpty(MessageType::COMMAND, "", "")).WillOnce(testing::Return(false));

    EXPECT_CALL(*mockQueue, getNext(MessageType::COMMAND, "", "")).WillOnce(testing::Return(testMessage));

    auto cmd = GetCommandFromQueue(mockQueue);

    ASSERT_EQ(cmd.has_value() ? cmd.value().Id : "", "112233");
    ASSERT_EQ(cmd.has_value() ? cmd.value().Command : "", "command_test");
    ASSERT_EQ(cmd.has_value() ? cmd.value().Parameters : nlohmann::json::object({"{}"}),
              R"({"parameters":["parameters_test"]})"_json);
    ASSERT_EQ(cmd.has_value() ? cmd.value().ExecutionResult.ErrorCode : module_command::Status::UNKNOWN,
              module_command::Status::IN_PROGRESS);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
