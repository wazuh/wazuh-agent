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

#include "queue.hpp"
#include "queue_test.hpp"

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

// TODO: Makes sense to add a full_clean method inside the persistence implementation?
void cleanPersistence()
{
    std::string filePath = DEFAULT_DB_PATH;
    for (const auto& entry : std::filesystem::directory_iterator("."))
    {
        std::string fileFullPath = entry.path();
        size_t found = fileFullPath.find(filePath);
        if (found != std::string::npos)
        {
            std::error_code ec;
            std::filesystem::remove(fileFullPath, ec);
        }
    }
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
TEST_F(QueueTest, SinglePushGetNotEmpty)
{
    MultiTypeQueue queue(BIG_QUEUE_CAPACITY);
    const MessageType messageType {MessageType::STATELESS};
    const Message messageToSend {messageType, baseDataContent};

    EXPECT_EQ(queue.timeoutPush(messageToSend), 1);
    auto messageResponse = queue.getLastMessage(MessageType::STATELESS);

    auto typeSend = messageToSend.type;
    auto typeReceived = messageResponse.type;
    EXPECT_TRUE(typeSend == typeReceived);

    auto dataResponse = messageResponse.data.at(0).at("data");
    EXPECT_EQ(dataResponse, baseDataContent);

    EXPECT_FALSE(queue.isEmptyByType(MessageType::STATELESS));
}

// timeoutPush and pop on a non-full queue
TEST_F(QueueTest, SinglePushPopEmpty)
{
    MultiTypeQueue queue(BIG_QUEUE_CAPACITY);
    const MessageType messageType {MessageType::STATELESS};
    const Message messageToSend {messageType, baseDataContent};

    EXPECT_EQ(queue.timeoutPush(messageToSend), 1);
    auto messageResponse = queue.getLastMessage(MessageType::STATELESS);
    auto dataResponse = messageResponse.data.at(0).at("data");
    EXPECT_EQ(dataResponse, baseDataContent);
    EXPECT_EQ(messageType, messageResponse.type);

    auto messageResponseStateFul = queue.getLastMessage(MessageType::STATEFUL);
    // TODO: this behavior can be change to return an empty message (type and module empty)
    EXPECT_EQ(messageResponseStateFul.type, MessageType::STATEFUL);
    EXPECT_EQ(messageResponseStateFul.data, "{}"_json);

    queue.popLastMessage(MessageType::STATELESS);
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATELESS));

    queue.popLastMessage(MessageType::STATELESS);
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATELESS));
}

TEST_F(QueueTest, SinglePushGetWithModule)
{
    MultiTypeQueue queue(BIG_QUEUE_CAPACITY);
    const MessageType messageType {MessageType::STATELESS};
    const std::string moduleFakeName = "fake-module";
    const std::string moduleName = "module";
    const Message messageToSend {messageType, baseDataContent, moduleName};

    EXPECT_EQ(queue.timeoutPush(messageToSend), 1);
    auto messageResponseWrongModule = queue.getLastMessage(MessageType::STATELESS, moduleFakeName);

    auto typeSend = messageToSend.type;
    auto typeReceived = messageResponseWrongModule.type;
    EXPECT_TRUE(typeSend == typeReceived);

    EXPECT_EQ(messageResponseWrongModule.moduleName, moduleFakeName);
    EXPECT_EQ(messageResponseWrongModule.data, "{}"_json);

    auto messageResponseCorrectModule = queue.getLastMessage(MessageType::STATELESS, moduleName);

    auto dataResponse = messageResponseCorrectModule.data.at(0).at("data");
    EXPECT_EQ(dataResponse, baseDataContent);

    EXPECT_EQ(moduleName, messageResponseCorrectModule.moduleName);
}

// Push, get and check while the queue is full
TEST_F(QueueTest, SinglePushPopFullWithTimeout)
{
    MultiTypeQueue queue(SMALL_QUEUE_CAPACITY);

    // complete the queue with messages
    const MessageType messageType {MessageType::COMMAND};
    for (int i : {1, 2})
    {
        const json dataContent = R"({"Data" : "for COMMAND)" + std::to_string(i) + R"("})";
        EXPECT_EQ(queue.timeoutPush({messageType, dataContent}), 1);
    }

    const json dataContent = R"({"Data" : "for COMMAND3"})";
    Message exampleMessage {messageType, dataContent};
    EXPECT_EQ(queue.timeoutPush({messageType, dataContent}, true), 0);

    auto items = queue.getItemsByType(MessageType::COMMAND);
    EXPECT_EQ(items, SMALL_QUEUE_CAPACITY);
    EXPECT_TRUE(queue.isFullByType(MessageType::COMMAND));
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATELESS));

    queue.popLastMessage(MessageType::COMMAND);
    items = queue.getItemsByType(MessageType::COMMAND);
    EXPECT_NE(items, SMALL_QUEUE_CAPACITY);
}

// Accesing different types of queues
TEST_F(QueueTest, MultithreadDifferentType)
{
    MultiTypeQueue queue(BIG_QUEUE_CAPACITY);

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
            EXPECT_EQ(queue.timeoutPush(Message(MessageType::STATELESS, dataContent)), 1);
            EXPECT_EQ(queue.timeoutPush(Message(MessageType::STATEFUL, dataContent)), 1);
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

    EXPECT_NE(0, queue.getItemsByType(MessageType::STATELESS));
    EXPECT_NE(0, queue.getItemsByType(MessageType::STATEFUL));
    EXPECT_FALSE(queue.isEmptyByType(MessageType::STATELESS));
    EXPECT_FALSE(queue.isEmptyByType(MessageType::STATEFUL));

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

    // FIXME: this doesn't match
    EXPECT_EQ(0, queue.getItemsByType(MessageType::STATELESS));
    EXPECT_EQ(0, queue.getItemsByType(MessageType::STATEFUL));
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATELESS));
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATEFUL));
}

// Accesing same queue
TEST_F(QueueTest, MultithreadSameType)
{
    MultiTypeQueue queue(BIG_QUEUE_CAPACITY);

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
            EXPECT_EQ(queue.timeoutPush(Message(MessageType::COMMAND, dataContent)), 1);
        }
    };

    int itemsToInsert = 10;
    int itemsToConsume = 5;

    messageProducer(itemsToInsert);

    EXPECT_EQ(itemsToInsert, queue.getItemsByType(MessageType::COMMAND));

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

    EXPECT_TRUE(queue.isEmptyByType(MessageType::COMMAND));
}

// Push Multiple with single message and data array,
// several gets, checks and pops
TEST_F(QueueTest, PushMultipleSeveralSingleGets)
{
    MultiTypeQueue queue(BIG_QUEUE_CAPACITY);
    const MessageType messageType {MessageType::STATELESS};
    // TODO: double check array of objects
    const Message messageToSend {messageType, multipleDataContent};

    EXPECT_EQ(3, queue.timeoutPush(messageToSend));

    for (int i : {0, 1, 2})
    {
        auto messageResponse = queue.getLastMessage(MessageType::STATELESS);
        auto responseData = messageResponse.data.at(0).at("data");
        auto sentData = messageToSend.data[i].template get<std::string>();
        EXPECT_EQ(responseData, sentData);
        queue.popLastMessage(MessageType::STATELESS);
    }

    EXPECT_EQ(queue.getItemsByType(MessageType::STATELESS), 0);
}

TEST_F(QueueTest, PushMultipleWithMessageVector)
{
    MultiTypeQueue queue(BIG_QUEUE_CAPACITY);

    std::vector<Message> messages;
    const MessageType messageType {MessageType::STATELESS};
    for (std::string i : {"0", "1", "2"})
    {
        const json multipleDataContent = {"content " + i};
        messages.push_back({messageType, multipleDataContent});
    }
    EXPECT_EQ(messages.size(), 3);
    EXPECT_EQ(3, queue.timeoutPush(messages));
    EXPECT_EQ(queue.getItemsByType(MessageType::STATELESS), 3);
}

// Push Multiple, pop multiples
TEST_F(QueueTest, PushMultipleGetMultiple)
{
    MultiTypeQueue queue(BIG_QUEUE_CAPACITY);
    const MessageType messageType {MessageType::STATELESS};
    const Message messageToSend {messageType, multipleDataContent};

    EXPECT_EQ(3, queue.timeoutPush(messageToSend));
    EXPECT_EQ(queue.getItemsByType(MessageType::STATELESS), 3);
    EXPECT_EQ(queue.popNMessages(MessageType::STATELESS, 1), 1);
    EXPECT_EQ(queue.popNMessages(MessageType::STATELESS, 3), 2);
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATELESS));
    EXPECT_EQ(0, queue.getItemsByType(MessageType::STATELESS));
}

// Push Multiple, pop multiples
TEST_F(QueueTest, PushMultipleGetMultipleWithModule)
{
    MultiTypeQueue queue(BIG_QUEUE_CAPACITY);
    const MessageType messageType {MessageType::STATELESS};
    const std::string moduleName = "testModule";
    const Message messageToSend {messageType, multipleDataContent, moduleName};

    EXPECT_EQ(3, queue.timeoutPush(messageToSend));

    // Altough we're asking for 10 messages only the availables are returned.
    auto messagesReceived = queue.getNMessages(MessageType::STATELESS, 10);
    int i = 3;
    for (auto singleMessage : messagesReceived)
    {
        EXPECT_EQ("content " + std::to_string(i--), singleMessage.data.at("data"));
    }

    EXPECT_EQ(0, queue.getItemsByType(MessageType::STATELESS, "fakemodule"));
    EXPECT_EQ(3, queue.getItemsByType(MessageType::STATELESS));
    EXPECT_EQ(3, queue.getItemsByType(MessageType::STATELESS, moduleName));
}

TEST_F(QueueTest, PushSinglesleGetMultipleWithModule)
{
    MultiTypeQueue queue(BIG_QUEUE_CAPACITY);

    for (std::string i : {"1", "2", "3", "4", "5"})
    {
        const MessageType messageType {MessageType::STATELESS};
        const json multipleDataContent = {"content-" + i};
        const std::string moduleName = "module-" + i;
        const Message messageToSend {messageType, multipleDataContent, moduleName};
        EXPECT_EQ(1, queue.timeoutPush(messageToSend));
    }

    auto messagesReceived = queue.getNMessages(MessageType::STATELESS, 10);
    EXPECT_EQ(5, messagesReceived.size());
    int i = 5;
    for (auto singleMessage : messagesReceived)
    {
        auto val = i--;
        EXPECT_EQ("content-" + std::to_string(val), singleMessage.data.at("data"));
        EXPECT_EQ("module-" + std::to_string(val), singleMessage.data.at("module"));
    }

    auto messageReceivedContent1 = queue.getNMessages(MessageType::STATELESS, 10, "module-1");
    EXPECT_EQ(1, messageReceivedContent1.size());
}
