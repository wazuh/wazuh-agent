#include <chrono>
#include <filesystem>
#include <future>
#include <iomanip>
#include <iostream>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <stop_token>
#include <thread>

#include <boost/asio.hpp>
#include <boost/asio/experimental/co_spawn.hpp>

#include "multitype_queue.hpp"
#include "multitype_queue_test.hpp"

using json = nlohmann::json;

#define BIG_QUEUE_CAPACITY   10
#define SMALL_QUEUE_CAPACITY 2

const json baseDataContent = R"({{"data": "for STATELESS_0"}})";
const json multipleDataContent = {"content 1", "content 2", "content 3"};

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

void cleanPersistence()
{
    for (const auto& entry : std::filesystem::directory_iterator("."))
    {
        const auto fileFullPath = entry.path().string();
        if (fileFullPath.find(QUEUE_DEFAULT_DB_PATH) != std::string::npos)
        {
            std::error_code ec;
            std::filesystem::remove(fileFullPath, ec);
        }
    }
}

/// Test Methods

void MultiTypeQueueTest::SetUp()
{
    cleanPersistence();
};

void MultiTypeQueueTest::TearDown() {};

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

    auto versionUj12 = uj1.at("version");
    auto versionUj22 = uj2.at("version");
    EXPECT_EQ(versionUj12, versionUj22);

    auto typeUj1 = uj1["type"].template get<std::string>();
    auto typeUj2 = uj2["type"].template get<std::string>();
    EXPECT_EQ(typeUj1, typeUj2);

    auto typeUj12 = uj1.at("type");
    auto typeUj22 = uj2.at("type");
    EXPECT_EQ(typeUj12, typeUj22);
}

TEST_F(JsonTest, JSONArrays)
{
    // create JSON values
    json j_object = {{"one", 1}, {"two", 2}, {"three", 3}};
    json j_array = {1, 2, 4, 8, 16};
    // TODO: test string: const std::string multipleDataContent = R"({"data": {"content 1", "content 2", "content 3"})";

    // call is_array()
    EXPECT_FALSE(j_object.is_array());
    EXPECT_TRUE(j_array.is_array());
    EXPECT_EQ(5, j_array.size());
    EXPECT_TRUE(multipleDataContent.is_array());

    int i = 0;
    for (auto& singleMessage : multipleDataContent.items())
    {
        EXPECT_EQ(singleMessage.value(), "content " + std::to_string(++i));
    }
}

// Push, get and check the queue is not empty
TEST_F(MultiTypeQueueTest, SinglePushGetNotEmpty)
{
    MultiTypeQueue multiTypeQueue(BIG_QUEUE_CAPACITY);
    const MessageType messageType {MessageType::STATELESS};
    const Message messageToSend {messageType, baseDataContent};

    EXPECT_EQ(multiTypeQueue.push(messageToSend), 1);
    auto messageResponse = multiTypeQueue.getNext(MessageType::STATELESS);

    auto typeSend = messageToSend.type;
    auto typeReceived = messageResponse.type;
    EXPECT_TRUE(typeSend == typeReceived);

    auto dataResponse = messageResponse.data.at(0).at("data");
    EXPECT_EQ(dataResponse, baseDataContent);

    EXPECT_FALSE(multiTypeQueue.isEmpty(MessageType::STATELESS));
}

// push and pop on a non-full queue
TEST_F(MultiTypeQueueTest, SinglePushPopEmpty)
{
    MultiTypeQueue multiTypeQueue(BIG_QUEUE_CAPACITY);
    const MessageType messageType {MessageType::STATELESS};
    const Message messageToSend {messageType, baseDataContent};

    EXPECT_EQ(multiTypeQueue.push(messageToSend), 1);
    auto messageResponse = multiTypeQueue.getNext(MessageType::STATELESS);
    auto dataResponse = messageResponse.data.at(0).at("data");
    EXPECT_EQ(dataResponse, baseDataContent);
    EXPECT_EQ(messageType, messageResponse.type);

    auto messageResponseStateFul = multiTypeQueue.getNext(MessageType::STATEFUL);
    // TODO: this behavior can be change to return an empty message (type and module empty)
    EXPECT_EQ(messageResponseStateFul.type, MessageType::STATEFUL);
    EXPECT_EQ(messageResponseStateFul.data, "{}"_json);

    multiTypeQueue.pop(MessageType::STATELESS);
    EXPECT_TRUE(multiTypeQueue.isEmpty(MessageType::STATELESS));

    multiTypeQueue.pop(MessageType::STATELESS);
    EXPECT_TRUE(multiTypeQueue.isEmpty(MessageType::STATELESS));
}

TEST_F(MultiTypeQueueTest, SinglePushGetWithModule)
{
    MultiTypeQueue multiTypeQueue(BIG_QUEUE_CAPACITY);
    const MessageType messageType {MessageType::STATELESS};
    const std::string moduleFakeName = "fake-module";
    const std::string moduleName = "module";
    const Message messageToSend {messageType, baseDataContent, moduleName};

    EXPECT_EQ(multiTypeQueue.push(messageToSend), 1);
    auto messageResponseWrongModule = multiTypeQueue.getNext(MessageType::STATELESS, moduleFakeName);

    auto typeSend = messageToSend.type;
    auto typeReceived = messageResponseWrongModule.type;
    EXPECT_TRUE(typeSend == typeReceived);

    EXPECT_EQ(messageResponseWrongModule.moduleName, moduleFakeName);
    EXPECT_EQ(messageResponseWrongModule.data, "{}"_json);

    auto messageResponseCorrectModule = multiTypeQueue.getNext(MessageType::STATELESS, moduleName);

    auto dataResponse = messageResponseCorrectModule.data.at(0).at("data");
    EXPECT_EQ(dataResponse, baseDataContent);

    EXPECT_EQ(moduleName, messageResponseCorrectModule.moduleName);
}

// Push, get and check while the queue is full
TEST_F(MultiTypeQueueTest, SinglePushPopFullWithTimeout)
{
    MultiTypeQueue multiTypeQueue(SMALL_QUEUE_CAPACITY);

    // complete the queue with messages
    const MessageType messageType {MessageType::COMMAND};
    for (int i : {1, 2})
    {
        const json dataContent = R"({"Data" : "for COMMAND)" + std::to_string(i) + R"("})";
        EXPECT_EQ(multiTypeQueue.push({messageType, dataContent}), 1);
    }

    const json dataContent = R"({"Data" : "for COMMAND3"})";
    Message exampleMessage {messageType, dataContent};
    EXPECT_EQ(multiTypeQueue.push({messageType, dataContent}, true), 0);

    auto items = multiTypeQueue.storedItems(MessageType::COMMAND);
    EXPECT_EQ(items, SMALL_QUEUE_CAPACITY);
    EXPECT_TRUE(multiTypeQueue.isFull(MessageType::COMMAND));
    EXPECT_TRUE(multiTypeQueue.isEmpty(MessageType::STATELESS));

    multiTypeQueue.pop(MessageType::COMMAND);
    items = multiTypeQueue.storedItems(MessageType::COMMAND);
    EXPECT_NE(items, SMALL_QUEUE_CAPACITY);
}

// Accesing different types of queues from several threads
TEST_F(MultiTypeQueueTest, MultithreadDifferentType)
{
    MultiTypeQueue multiTypeQueue(BIG_QUEUE_CAPACITY);

    auto consumerStateLess = [&](int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            multiTypeQueue.pop(MessageType::STATELESS);
        }
    };

    auto consumerStateFull = [&](int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            multiTypeQueue.pop(MessageType::STATEFUL);
        }
    };

    auto messageProducer = [&](int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            const json dataContent = R"({{"Data", "Number )" + std::to_string(i) + R"("}})";
            EXPECT_EQ(multiTypeQueue.push(Message(MessageType::STATELESS, dataContent)), 1);
            EXPECT_EQ(multiTypeQueue.push(Message(MessageType::STATEFUL, dataContent)), 1);
        }
    };

    int itemsToInsert = 10;
    int itemsToConsume = 5;

    messageProducer(itemsToInsert);

    std::thread consumerThread1(consumerStateLess, std::ref(itemsToConsume));
    std::thread consumerThread2(consumerStateFull, std::ref(itemsToConsume));

    if (consumerThread1.joinable())
    {
        consumerThread1.join();
    }

    if (consumerThread2.joinable())
    {
        consumerThread2.join();
    }

    EXPECT_EQ(5, multiTypeQueue.storedItems(MessageType::STATELESS));
    EXPECT_EQ(5, multiTypeQueue.storedItems(MessageType::STATEFUL));

    // Consume the rest of the messages
    std::thread consumerThread12(consumerStateLess, std::ref(itemsToConsume));
    std::thread consumerThread22(consumerStateFull, std::ref(itemsToConsume));

    if (consumerThread12.joinable())
    {
        consumerThread12.join();
    }

    if (consumerThread22.joinable())
    {
        consumerThread22.join();
    }

    EXPECT_TRUE(multiTypeQueue.isEmpty(MessageType::STATELESS));
    EXPECT_TRUE(multiTypeQueue.isEmpty(MessageType::STATEFUL));
}

// Accesing same queue from 2 different threads
TEST_F(MultiTypeQueueTest, MultithreadSameType)
{
    MultiTypeQueue multiTypeQueue(BIG_QUEUE_CAPACITY);
    auto messageType = MessageType::COMMAND;

    auto consumerCommand1 = [&](int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            multiTypeQueue.pop(messageType);
        }
    };

    auto consumerCommand2 = [&](int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            multiTypeQueue.pop(messageType);
        }
    };

    auto messageProducer = [&](int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            const json dataContent = R"({{"Data": "for COMMAND)" + std::to_string(i) + R"("}})";
            EXPECT_EQ(multiTypeQueue.push(Message(messageType, dataContent)), 1);
        }
    };

    int itemsToInsert = 10;
    int itemsToConsume = 5;

    messageProducer(itemsToInsert);

    EXPECT_EQ(itemsToInsert, multiTypeQueue.storedItems(messageType));

    std::thread consumerThread1(consumerCommand1, std::ref(itemsToConsume));
    std::thread messageProducerThread1(consumerCommand2, std::ref(itemsToConsume));

    if (messageProducerThread1.joinable())
    {
        messageProducerThread1.join();
    }

    if (consumerThread1.joinable())
    {
        consumerThread1.join();
    }

    EXPECT_TRUE(multiTypeQueue.isEmpty(messageType));
}

// Push Multiple with single message and data array,
// several gets, checks and pops
TEST_F(MultiTypeQueueTest, PushMultipleSeveralSingleGets)
{
    MultiTypeQueue multiTypeQueue(BIG_QUEUE_CAPACITY);
    const MessageType messageType {MessageType::STATELESS};
    const Message messageToSend {messageType, multipleDataContent};

    EXPECT_EQ(3, multiTypeQueue.push(messageToSend));

    for (int i : {0, 1, 2})
    {
        auto messageResponse = multiTypeQueue.getNext(MessageType::STATELESS);
        auto responseData = messageResponse.data.at(0).at("data");
        auto sentData = messageToSend.data[i].template get<std::string>();
        EXPECT_EQ(responseData, sentData);
        multiTypeQueue.pop(MessageType::STATELESS);
    }

    EXPECT_EQ(multiTypeQueue.storedItems(MessageType::STATELESS), 0);
}

TEST_F(MultiTypeQueueTest, PushMultipleWithMessageVector)
{
    MultiTypeQueue multiTypeQueue(BIG_QUEUE_CAPACITY);

    std::vector<Message> messages;
    const MessageType messageType {MessageType::STATELESS};
    for (std::string i : {"0", "1", "2"})
    {
        const json multipleDataContent = {"content " + i};
        messages.push_back({messageType, multipleDataContent});
    }
    EXPECT_EQ(messages.size(), 3);
    EXPECT_EQ(multiTypeQueue.push(messages), 3);
    EXPECT_EQ(multiTypeQueue.storedItems(MessageType::STATELESS), 3);
}

// push message vector with a mutiple data element
TEST_F(MultiTypeQueueTest, PushVectorWithAMultipleInside)
{
    MultiTypeQueue multiTypeQueue(BIG_QUEUE_CAPACITY);

    std::vector<Message> messages;

    // triple data content message
    const MessageType messageType {MessageType::STATELESS};
    const Message messageToSend {messageType, multipleDataContent};
    messages.push_back(messageToSend);

    // triple message vector
    for (std::string i : {"0", "1", "2"})
    {
        const json dataContent = {"content " + i};
        messages.push_back({messageType, dataContent});
    }

    EXPECT_EQ(6, multiTypeQueue.push(messages));
}

// Push Multiple, pop multiples
TEST_F(MultiTypeQueueTest, PushMultipleGetMultiple)
{
    MultiTypeQueue multiTypeQueue(BIG_QUEUE_CAPACITY);
    const MessageType messageType {MessageType::STATELESS};
    const Message messageToSend {messageType, multipleDataContent};

    EXPECT_EQ(3, multiTypeQueue.push(messageToSend));
    EXPECT_EQ(multiTypeQueue.storedItems(MessageType::STATELESS), 3);
    EXPECT_EQ(multiTypeQueue.popN(MessageType::STATELESS, 1), 1);
    EXPECT_EQ(multiTypeQueue.popN(MessageType::STATELESS, 3), 2);
    EXPECT_TRUE(multiTypeQueue.isEmpty(MessageType::STATELESS));
    EXPECT_EQ(0, multiTypeQueue.storedItems(MessageType::STATELESS));
}

// Push Multiple, pop multiples
TEST_F(MultiTypeQueueTest, PushMultipleGetMultipleWithModule)
{
    MultiTypeQueue multiTypeQueue(BIG_QUEUE_CAPACITY);
    const MessageType messageType {MessageType::STATELESS};
    const std::string moduleName = "testModule";
    const Message messageToSend {messageType, multipleDataContent, moduleName};

    EXPECT_EQ(3, multiTypeQueue.push(messageToSend));

    // Altough we're asking for 10 messages only the availables are returned.
    auto messagesReceived = multiTypeQueue.getNextN(MessageType::STATELESS, 10);
    int i = 0;
    for (auto singleMessage : messagesReceived)
    {
        EXPECT_EQ("content " + std::to_string(++i), singleMessage.data.at("data").get<std::string>());
    }

    EXPECT_EQ(0, multiTypeQueue.storedItems(MessageType::STATELESS, "fakemodule"));
    EXPECT_EQ(3, multiTypeQueue.storedItems(MessageType::STATELESS));
    EXPECT_EQ(3, multiTypeQueue.storedItems(MessageType::STATELESS, moduleName));
}

TEST_F(MultiTypeQueueTest, PushSinglesleGetMultipleWithModule)
{
    MultiTypeQueue multiTypeQueue(BIG_QUEUE_CAPACITY);

    for (std::string i : {"1", "2", "3", "4", "5"})
    {
        const MessageType messageType {MessageType::STATELESS};
        const json multipleDataContent = {"content-" + i};
        const std::string moduleName = "module-" + i;
        const Message messageToSend {messageType, multipleDataContent, moduleName};
        EXPECT_EQ(1, multiTypeQueue.push(messageToSend));
    }

    auto messagesReceived = multiTypeQueue.getNextN(MessageType::STATELESS, 10);
    EXPECT_EQ(5, messagesReceived.size());
    int i = 0;
    for (auto singleMessage : messagesReceived)
    {
        auto val = ++i;
        EXPECT_EQ("content-" + std::to_string(val), singleMessage.data.at("data").get<std::string>());
        EXPECT_EQ("module-" + std::to_string(val), singleMessage.data.at("module").get<std::string>());
    }

    auto messageReceivedContent1 = multiTypeQueue.getNextN(MessageType::STATELESS, 10, "module-1");
    EXPECT_EQ(1, messageReceivedContent1.size());
}

TEST_F(MultiTypeQueueTest, GetNextAwaitableBase)
{
    MultiTypeQueue multiTypeQueue(BIG_QUEUE_CAPACITY);
    boost::asio::io_context io_context;

    // Coroutine that waits till there's a message of the needed type on the queue
    boost::asio::co_spawn(
        io_context,
        [&multiTypeQueue]() -> boost::asio::awaitable<void>
        {
            auto messageReceived = co_await multiTypeQueue.getNextNAwaitable(MessageType::STATELESS, 2);
            EXPECT_EQ(messageReceived.data.at(0).at("data"), "content-1");
            EXPECT_EQ(messageReceived.data.at(1).at("data"), "content-2");
        },
        boost::asio::detached);

    // Simulate the addition of needed message to the queue after some time
    std::thread producer(
        [&multiTypeQueue, &io_context]()
        {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            const MessageType messageType {MessageType::STATELESS};
            const json multipleDataContent = {"content-1", "content-2", "content-3"};
            const Message messageToSend {messageType, multipleDataContent};
            EXPECT_EQ(multiTypeQueue.push(messageToSend), 3);
            // io_context.stop();
        });

    io_context.run();
    producer.join();
}

TEST_F(MultiTypeQueueTest, PushAwaitable)
{
    MultiTypeQueue multiTypeQueue(SMALL_QUEUE_CAPACITY);
    boost::asio::io_context io_context;

    // complete the queue with messages
    const MessageType messageType {MessageType::STATEFUL};
    for (int i : {1, 2})
    {
        const json dataContent = R"({"Data" : "for STATEFUL)" + std::to_string(i) + R"("})";
        EXPECT_EQ(multiTypeQueue.push({messageType, dataContent}), 1);
    }

    EXPECT_TRUE(multiTypeQueue.isFull(MessageType::STATEFUL));
    EXPECT_EQ(multiTypeQueue.storedItems(MessageType::STATEFUL), 2);

    // Coroutine that waits till there's space to push a new message
    boost::asio::co_spawn(
        io_context,
        [&multiTypeQueue]() -> boost::asio::awaitable<void>
        {
            const MessageType messageType {MessageType::STATEFUL};
            const json dataContent = {"content-1"};
            const Message messageToSend {messageType, dataContent};
            EXPECT_EQ(multiTypeQueue.storedItems(MessageType::STATEFUL), 2);
            auto messagesPushed = co_await multiTypeQueue.pushAwaitable(messageToSend);
            EXPECT_EQ(messagesPushed, 1);
            EXPECT_EQ(multiTypeQueue.storedItems(MessageType::STATEFUL), 2);
        },
        boost::asio::detached);

    // Simulate poping one message so there's space to push a new one
    std::thread consumer(
        [&multiTypeQueue, &io_context]()
        {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            EXPECT_EQ(multiTypeQueue.popN(MessageType::STATEFUL, 1), 1);
            // TODO: double check this behavior, is it mandatory to stop the context here?
            // io_context.stop();
        });

    io_context.run();
    consumer.join();

    EXPECT_TRUE(multiTypeQueue.isFull(MessageType::STATEFUL));
}

TEST_F(MultiTypeQueueTest, FifoOrderCheck)
{
    MultiTypeQueue multiTypeQueue(BIG_QUEUE_CAPACITY);

    // complete the queue with messages
    const MessageType messageType {MessageType::STATEFUL};
    for (int i : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10})
    {
        const json dataContent = R"({"Data" : "for STATEFUL)" + std::to_string(i) + R"("})";
        EXPECT_EQ(multiTypeQueue.push({messageType, dataContent}), 1);
    }

    auto messageReceivedVector = multiTypeQueue.getNextN(messageType, 10);
    EXPECT_EQ(messageReceivedVector.size(), 10);
    int i = 0;
    for (auto singleMessage : messageReceivedVector)
    {
        EXPECT_EQ(singleMessage.data.at("data"), R"({"Data" : "for STATEFUL)" + std::to_string(++i) + R"("})");
    }

    // Keep the order of the message: FIFO
    for (int i : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10})
    {
        auto messageReceived = multiTypeQueue.getNextN(messageType, 1);
        EXPECT_EQ(messageReceived.at(0).data.at("data"), R"({"Data" : "for STATEFUL)" + std::to_string(i) + R"("})");
        EXPECT_TRUE(multiTypeQueue.pop(messageType));
    }
}
