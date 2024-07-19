#include <chrono>
#include <filesystem>
#include <future>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <stop_token>
#include <thread>

#include "queue.hpp"
#include "queue_test.hpp"

using json = nlohmann::json;

#define BIG_QUEUE_QTTY   10
#define SMALL_QUEUE_QTTY 2

const json baseDataContent = R"({{"data": "for STATELESS_0"}})";
const json multipleDataContent = R"({{"content 1", "content 2", "content 3"}})";
// TODO: test string: const std::string multipleDataContent = R"({"data": {"content 1", "content 2", "content 3"})";

/// Helper functions

// Unescape Strings
std::string unescape_string(const std::string& str)
{
    std::string result;
    result.reserve(str.length());

    for (size_t i = 0; i < str.length(); ++i)
    {
        if (str[i] == '\\' && i + 1 < str.length())
        {
            switch (str[i + 1])
            {
                case '\\': result += '\\'; break;
                case '\"': result += '\"'; break;
                case '/': result += '/'; break;
                case 'b': result += '\b'; break;
                case 'f': result += '\f'; break;
                case 'n': result += '\n'; break;
                case 'r': result += '\r'; break;
                case 't': result += '\t'; break;
                default: result += str[i + 1];
            }
            ++i;
        }
        else
        {
            result += str[i];
        }
    }

    return result;
}

// TODO: Makes sense to add a full_clean method inside the persistence implementation?
void cleanPersistence()
{
    auto filePath = DEFAULT_DB_PATH;
    std::error_code ec;
    std::filesystem::remove(filePath, ec);
}

/// Test Methods

void QueueTest::SetUp()
{
    cleanPersistence();
};

void QueueTest::TearDown() {};

/// TESTS

// JSON Basic methods. Move or delete if JSON Wrapper is done
TEST_F(JsonTest, JSONConversionComparisson)
{
    json uj1 = {{"version", 1}, {"type", "integer"}};
    // From string. If not unescape then it throws errors
    json uj2 = json::parse(unescape_string(R"({\"type\":\"integer\",\"version\":1})"));

    nlohmann::ordered_json oj1 = {{"version", 1}, {"type", "integer"}};
    nlohmann::ordered_json oj2 = {{"type", "integer"}, {"version", 1}};
    EXPECT_FALSE(oj1 == oj2);

    auto versionUj1 = uj1["version"].template get<int>();
    auto versionUj2 = uj2["version"].template get<int>();
    EXPECT_EQ(versionUj1, versionUj2);

    auto typeUj1 = uj1["type"].template get<std::string>();
    auto typeUj2 = uj2["type"].template get<std::string>();
    EXPECT_EQ(typeUj1, typeUj2);
}

TEST_F(JsonTest, JSONArrays)
{
    // create JSON values
    json j_object = {{"one", 1}, {"two", 2}, {"three", 3}};
    json j_array = {1, 2, 4, 8, 16};
    json multipleDataContent = {"content 1", "content 2", "content 3"};

    // call is_array()
    EXPECT_FALSE(j_object.is_array());
    EXPECT_TRUE(j_array.is_array());
    EXPECT_EQ(5, j_array.size());
    EXPECT_TRUE(multipleDataContent.is_array());

    int i = 0;
    for (auto& singleMessage : multipleDataContent.items())
    {
        EXPECT_EQ(singleMessage.value(),"content " + std::to_string(++i));
    }
}

// Push, get and check the queue is not empty
TEST_F(QueueTest, SinglePushGetNotEmpty)
{
    MultiTypeQueue queue(BIG_QUEUE_QTTY);
    const MessageType messageType {MessageType::STATELESS};
    const Message messageToSend {messageType, baseDataContent};

    queue.timeoutPush(messageToSend);
    auto messageResponse = queue.getLastMessage(MessageType::STATELESS);

    auto typeSend = messageToSend.type;
    auto typeReceived = messageResponse.type;
    EXPECT_TRUE(typeSend == typeReceived);

    auto dataResponse = messageResponse.data[0].template get<std::string>();
    auto dataToSend = messageToSend.data.template get<std::string>();
    EXPECT_EQ(dataResponse, dataToSend);

    EXPECT_FALSE(queue.isEmptyByType(MessageType::STATELESS));
}

// timeoutPush and pop on a non-full queue
TEST_F(QueueTest, SinglePushPopEmpty)
{
    MultiTypeQueue queue(BIG_QUEUE_QTTY);
    const MessageType messageType {MessageType::STATELESS};
    const Message messageToSend {messageType, baseDataContent};

    queue.timeoutPush(messageToSend);
    auto messageResponse = queue.getLastMessage(MessageType::STATELESS);

    auto typeSend = messageToSend.type;
    auto typeReceived = messageResponse.type;
    EXPECT_TRUE(typeSend == typeReceived);

    auto dataResponse = messageResponse.data[0].template get<std::string>();
    auto dataToSend = messageToSend.data.template get<std::string>();
    EXPECT_EQ(dataResponse, dataToSend);

    queue.popLastMessage(MessageType::STATELESS);
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATELESS));
}

// Push, pop and check the queue is empty
TEST_F(QueueTest, SinglePushPop)
{
    MultiTypeQueue queue(BIG_QUEUE_QTTY);
    const MessageType messageType {MessageType::STATELESS};
    const Message messageToSend {messageType, baseDataContent};

    queue.timeoutPush(messageToSend);
    EXPECT_FALSE(queue.isEmptyByType(MessageType::STATELESS));

    queue.popLastMessage(MessageType::STATELESS);
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATELESS));
}

// get and pop on a empty queue
TEST_F(QueueTest, SingleGetPopOnEmpty)
{
    MultiTypeQueue queue(BIG_QUEUE_QTTY);
    const MessageType messageType {MessageType::STATELESS};
    const Message messageToSend {messageType, baseDataContent};

    queue.timeoutPush(messageToSend);
    EXPECT_FALSE(queue.isEmptyByType(MessageType::STATELESS));
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATEFUL));

    auto messageResponse = queue.getLastMessage(MessageType::STATEFUL);
    EXPECT_EQ(messageResponse.type, MessageType::STATEFUL);
    EXPECT_EQ(messageResponse.data[0], nullptr);

    queue.popLastMessage(MessageType::STATEFUL);
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATEFUL));
}

// Push, get and check while the queue is full
TEST_F(QueueTest, SinglePushPopFullWithTimeout)
{
    MultiTypeQueue queue(SMALL_QUEUE_QTTY);

    // complete the queue with messages
    const MessageType messageType {MessageType::COMMAND};
    for (int i : {1, 2})
    {
        const json dataContent = R"({"Data" : "for COMMAND)" + std::to_string(i) + R"("})";
        queue.timeoutPush({messageType, dataContent});
    }

    const json dataContent = R"({"Data" : "for COMMAND3"})";
    Message exampleMessage {messageType, dataContent};
    queue.timeoutPush({messageType, dataContent},true);

    auto items = queue.getItemsByType(MessageType::COMMAND);
    EXPECT_EQ(items, SMALL_QUEUE_QTTY);
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATELESS));

    queue.popLastMessage(MessageType::COMMAND);
    items = queue.getItemsByType(MessageType::COMMAND);
    EXPECT_NE(items, SMALL_QUEUE_QTTY);
}

// Accesing different types of queues
TEST_F(QueueTest, MultithreadDifferentType)
{
    GTEST_SKIP();
    MultiTypeQueue queue(BIG_QUEUE_QTTY);

    auto consumerStateLess = [&](int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            queue.popLastMessage(MessageType::STATELESS);
        }
    };

    auto consumerStateFull = [&](int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            queue.popLastMessage(MessageType::STATEFUL);
        }
    };

    auto messageProducer = [&](int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            const json dataContent = R"({{"Data", "for STATELESS)" + std::to_string(i) + R"("}})";
            queue.timeoutPush(Message(MessageType::STATELESS, dataContent));
            queue.timeoutPush(Message(MessageType::STATEFUL, dataContent));
        }
    };

    int itemsToInsert = 10;
    int itemsToConsume = 5;

    messageProducer(itemsToInsert);

    std::thread consumerThread1(consumerStateLess, std::ref(itemsToConsume));
    std::thread consumerThread2(consumerStateFull, std::ref(itemsToConsume));

    if(consumerThread1.joinable())
    {
        consumerThread1.join();
    }

    if(consumerThread2.joinable())
    {
        consumerThread2.join();
    }


    EXPECT_NE(0, queue.getItemsByType(MessageType::STATELESS));
    EXPECT_NE(0, queue.getItemsByType(MessageType::STATEFUL));
    EXPECT_FALSE(queue.isEmptyByType(MessageType::STATELESS));
    EXPECT_FALSE(queue.isEmptyByType(MessageType::STATEFUL));

    // Consume the rest of the messages
    std::thread consumerThread12(consumerStateLess, std::ref(itemsToConsume));
    std::thread consumerThread22(consumerStateFull, std::ref(itemsToConsume));

    if(consumerThread12.joinable())
    {
        consumerThread12.join();
    }

    if(consumerThread22.joinable())
    {
        consumerThread22.join();
    }


    // FIXME: this doesn't match
    EXPECT_EQ(0, queue.getItemsByType(MessageType::STATELESS));
    EXPECT_EQ(0, queue.getItemsByType(MessageType::STATEFUL));
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATELESS));
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATEFUL));
}

// Accesing same queue
TEST_F(QueueTest, MultithreadSameType)
{
    MultiTypeQueue queue(BIG_QUEUE_QTTY);

    auto consumerCommand1 = [&](int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            queue.popLastMessage(MessageType::COMMAND);
        }
    };

    auto consumerCommand2 = [&](int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            queue.popLastMessage(MessageType::COMMAND);
        }
    };

    auto messageProducer = [&](int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            const json dataContent = R"({{"Data": "for COMMAND)" + std::to_string(i) + R"("}})";
            queue.timeoutPush(Message(MessageType::COMMAND, dataContent));
        }
    };

    int itemsToInsert = 10;
    int itemsToConsume = 5;

    messageProducer(itemsToInsert);

    EXPECT_EQ(itemsToInsert, queue.getItemsByType(MessageType::COMMAND));

    std::thread consumerThread1(consumerCommand1, std::ref(itemsToConsume));
    std::thread messageProducerThread1(consumerCommand2, std::ref(itemsToConsume));

    if(messageProducerThread1.joinable())
    {
        messageProducerThread1.join();
    }

    if(consumerThread1.joinable())
    {
        consumerThread1.join();
    }

    EXPECT_TRUE(queue.isEmptyByType(MessageType::COMMAND));
}

// Push Multiple with single message and data array,
// several gets, checks and pops
TEST_F(QueueTest, MultiplePushSeveralSingleGets)
{
    MultiTypeQueue queue(BIG_QUEUE_QTTY);
    const MessageType messageType {MessageType::STATELESS};
    // TODO: double check array of objects
    const json multipleDataContent = {"content 1", "content 2", "content 3"};
    const Message messageToSend {messageType, multipleDataContent};

    queue.timeoutPush(messageToSend);

    for (int i : {0, 1, 2})
    {
        auto messageResponse = queue.getLastMessage(MessageType::STATELESS);
        auto responseData = messageResponse.data[0].template get<std::string>();
        auto sentData = messageToSend.data[i].template get<std::string>();
        EXPECT_EQ(responseData, sentData);
        queue.popLastMessage(MessageType::STATELESS);
    }

    EXPECT_EQ(queue.getItemsByType(MessageType::STATELESS), 0);
}

// Push Multiple, pop multiples
TEST_F(QueueTest, MultiplePushSeveralMultiplePops)
{
    MultiTypeQueue queue(BIG_QUEUE_QTTY);
    const MessageType messageType {MessageType::STATELESS};
    const json multipleDataContent = {"content 1", "content 2", "content 3"};
    const Message messageToSend {messageType, multipleDataContent};

    queue.timeoutPush(messageToSend);

    EXPECT_TRUE(queue.popNMessages(MessageType::STATELESS, 3));
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATELESS));
    EXPECT_EQ(0,queue.getItemsByType(MessageType::STATELESS));
}
