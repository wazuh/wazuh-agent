#include <algorithm>
#include <chrono>
#include <filesystem>
#include <future>
#include <iomanip>
#include <regex>
#include <sstream>
#include <stdexcept>
#include <thread>

#include <boost/asio.hpp>

#include <multitype_queue.hpp>
#include <storage.hpp>

#include "multitype_queue_test.hpp"

const std::string QUEUE_DB_NAME = "queue.db";
constexpr size_t SMALL_QUEUE_CAPACITY = 1000;
const nlohmann::json BASE_DATA_CONTENT = R"({{"data": "for STATELESS_0"}})";
const nlohmann::json MULTIPLE_DATA_CONTENT = {"content 1", "content 2", "content 3"};

namespace
{
    std::string UnescapeString(const std::string& str)
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

    void CleanPersistence()
    {
        for (const auto& entry : std::filesystem::directory_iterator("."))
        {
            const auto fileFullPath = entry.path().string();
            if (fileFullPath.find(QUEUE_DB_NAME) != std::string::npos)
            {
                std::error_code ec;
                std::filesystem::remove(fileFullPath, ec);
            }
        }
    }

    const auto MOCK_CONFIG_PARSER = std::make_shared<configuration::ConfigurationParser>(std::string(R"(
        agent:
          path.data: "."
    )"));

    const auto MOCK_CONFIG_PARSER_SMALL_SIZE = std::make_shared<configuration::ConfigurationParser>(std::string(R"(
        agent:
          path.data: "."
          queue_size: 1000
    )"));
} // namespace

/// Test Methods

void MultiTypeQueueTest::SetUp()
{
    CleanPersistence();
};

void MultiTypeQueueTest::TearDown() {};

/// TESTS

// JSON Basic methods. Move or delete if JSON Wrapper is done
TEST_F(JsonTest, JSONConversionComparisson)
{
    const nlohmann::json uj1 = {{"version", 1}, {"type", "integer"}};

    // From string. If not unescape then it throws errors
    const auto uj2 = nlohmann::json::parse(UnescapeString(R"({\"type\":\"integer\",\"version\":1})"));

    const nlohmann::ordered_json oj1 = {{"version", 1}, {"type", "integer"}};
    const nlohmann::ordered_json oj2 = {{"type", "integer"}, {"version", 1}};
    EXPECT_FALSE(oj1 == oj2);

    auto versionUj1 = uj1["version"].template get<int>();
    auto versionUj2 = uj2["version"].template get<int>();
    EXPECT_EQ(versionUj1, versionUj2);

    const auto& versionUj12 = uj1.at("version");
    const auto& versionUj22 = uj2.at("version");
    EXPECT_EQ(versionUj12, versionUj22);

    auto typeUj1 = uj1["type"].template get<std::string>();
    auto typeUj2 = uj2["type"].template get<std::string>();
    EXPECT_EQ(typeUj1, typeUj2);

    const auto& typeUj12 = uj1.at("type");
    const auto& typeUj22 = uj2.at("type");
    EXPECT_EQ(typeUj12, typeUj22);
}

TEST_F(JsonTest, JSONArrays)
{
    // create JSON values
    const nlohmann::json j_object = {{"one", 1}, {"two", 2}, {"three", 3}};
    const nlohmann::json j_array = {1, 2, 4, 8, 16};
    // TODO: test string: const std::string multipleDataContent = R"({"data": {"content 1", "content 2", "content 3"})";

    // call is_array()
    EXPECT_FALSE(j_object.is_array());
    EXPECT_TRUE(j_array.is_array());
    EXPECT_EQ(5, j_array.size());
    EXPECT_TRUE(MULTIPLE_DATA_CONTENT.is_array());

    int i = 0;
    for (auto& singleMessage : MULTIPLE_DATA_CONTENT.items())
    {
        EXPECT_EQ(singleMessage.value(), "content " + std::to_string(++i));
    }
}

TEST_F(MultiTypeQueueTest, Constructor)
{
    auto configurationParser = std::make_shared<configuration::ConfigurationParser>();

    EXPECT_NO_THROW(MultiTypeQueue multiTypeQueue(configurationParser));
}

TEST_F(MultiTypeQueueTest, ConstructorNoConfigParser)
{
    EXPECT_THROW(MultiTypeQueue multiTypeQueue(nullptr), std::runtime_error);
}

// Push, get and check the queue is not empty
TEST_F(MultiTypeQueueTest, SinglePushGetNotEmpty)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER);
    const MessageType messageType {MessageType::STATELESS};
    const Message messageToSend {messageType, BASE_DATA_CONTENT};

    EXPECT_EQ(multiTypeQueue.push(messageToSend), 1);
    auto messageResponse = multiTypeQueue.getNext(MessageType::STATELESS);

    auto typeSend = messageToSend.type;
    auto typeReceived = messageResponse.type;
    EXPECT_TRUE(typeSend == typeReceived);

    auto dataResponse = messageResponse.data;
    EXPECT_EQ(dataResponse, BASE_DATA_CONTENT);

    EXPECT_FALSE(multiTypeQueue.isEmpty(MessageType::STATELESS));
}

// push and pop on a non-full queue
TEST_F(MultiTypeQueueTest, SinglePushPopEmpty)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER);
    const MessageType messageType {MessageType::STATELESS};
    const Message messageToSend {messageType, BASE_DATA_CONTENT};

    EXPECT_EQ(multiTypeQueue.push(messageToSend), 1);
    auto messageResponse = multiTypeQueue.getNext(MessageType::STATELESS);
    auto dataResponse = messageResponse.data;
    EXPECT_EQ(dataResponse, BASE_DATA_CONTENT);
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
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER);
    const MessageType messageType {MessageType::STATELESS};
    const std::string moduleFakeName = "fake-module";
    const std::string moduleName = "module";
    const Message messageToSend {messageType, BASE_DATA_CONTENT, moduleName};

    EXPECT_EQ(multiTypeQueue.push(messageToSend), 1);
    auto messageResponseWrongModule = multiTypeQueue.getNext(MessageType::STATELESS, moduleFakeName);

    auto typeSend = messageToSend.type;
    auto typeReceived = messageResponseWrongModule.type;
    EXPECT_TRUE(typeSend == typeReceived);

    EXPECT_EQ(messageResponseWrongModule.moduleName, moduleFakeName);
    EXPECT_EQ(messageResponseWrongModule.data, "{}"_json);

    auto messageResponseCorrectModule = multiTypeQueue.getNext(MessageType::STATELESS, moduleName);

    auto dataResponse = messageResponseCorrectModule.data;
    EXPECT_EQ(dataResponse, BASE_DATA_CONTENT);

    EXPECT_EQ(moduleName, messageResponseCorrectModule.moduleName);
}

// Push, get and check while the queue is full
TEST_F(MultiTypeQueueTest, SinglePushPopFullWithTimeout)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER_SMALL_SIZE);

    // complete the queue with messages
    const MessageType messageType {MessageType::COMMAND};
    int upperLimit = SMALL_QUEUE_CAPACITY + 1;
    for (int i : std::views::iota(1, upperLimit))
    {
        const nlohmann::json dataContent = R"({"Data" : "for COMMAND)" + std::to_string(i) + R"("})";
        EXPECT_EQ(multiTypeQueue.push({messageType, dataContent}), 1);
    }

    const nlohmann::json dataContent = R"({"Data" : "for COMMAND3"})";
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
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER);

    auto consumerStateLess = [&](const int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            multiTypeQueue.pop(MessageType::STATELESS);
        }
    };

    auto consumerStateFull = [&](const int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            multiTypeQueue.pop(MessageType::STATEFUL);
        }
    };

    auto messageProducer = [&](const int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            const nlohmann::json dataContent = R"({{"Data", "Number )" + std::to_string(i) + R"("}})";
            EXPECT_EQ(multiTypeQueue.push(Message(MessageType::STATELESS, dataContent)), 1);
            EXPECT_EQ(multiTypeQueue.push(Message(MessageType::STATEFUL, dataContent)), 1);
        }
    };

    const int itemsToInsert = 10;
    const int itemsToConsume = 5;

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
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER);
    auto messageType = MessageType::COMMAND;

    auto consumerCommand1 = [&](const int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            multiTypeQueue.pop(messageType);
        }
    };

    auto consumerCommand2 = [&](const int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            multiTypeQueue.pop(messageType);
        }
    };

    auto messageProducer = [&](const int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            const nlohmann::json dataContent = R"({{"Data": "for COMMAND)" + std::to_string(i) + R"("}})";
            EXPECT_EQ(multiTypeQueue.push(Message(messageType, dataContent)), 1);
        }
    };

    const int itemsToInsert = 10;
    const int itemsToConsume = 5;

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
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER);
    const MessageType messageType {MessageType::STATELESS};
    const Message messageToSend {messageType, MULTIPLE_DATA_CONTENT};

    EXPECT_EQ(3, multiTypeQueue.push(messageToSend));

    for (size_t i : {0u, 1u, 2u})
    {
        auto messageResponse = multiTypeQueue.getNext(MessageType::STATELESS);
        auto responseData = messageResponse.data;
        auto sentData = messageToSend.data[i].template get<std::string>();
        EXPECT_EQ(responseData, sentData);
        multiTypeQueue.pop(MessageType::STATELESS);
    }

    EXPECT_EQ(multiTypeQueue.storedItems(MessageType::STATELESS), 0);
}

TEST_F(MultiTypeQueueTest, PushMultipleWithMessageVector)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER);

    std::vector<Message> messages;
    const MessageType messageType {MessageType::STATELESS};
    for (std::string i : {"0", "1", "2"})
    {
        const nlohmann::json multipleDataContent = {"content " + i};
        messages.emplace_back(messageType, multipleDataContent);
    }
    EXPECT_EQ(messages.size(), 3);
    EXPECT_EQ(multiTypeQueue.push(messages), 3);
    EXPECT_EQ(multiTypeQueue.storedItems(MessageType::STATELESS), 3);
}

// push message vector with a mutiple data element
TEST_F(MultiTypeQueueTest, PushVectorWithAMultipleInside)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER);

    std::vector<Message> messages;

    // triple data content message
    const MessageType messageType {MessageType::STATELESS};
    const Message messageToSend {messageType, MULTIPLE_DATA_CONTENT};
    messages.push_back(messageToSend);

    // triple message vector
    for (std::string i : {"0", "1", "2"})
    {
        const nlohmann::json dataContent = {"content " + i};
        messages.emplace_back(messageType, dataContent);
    }

    EXPECT_EQ(6, multiTypeQueue.push(messages));
}

// Push Multiple, pop multiples
TEST_F(MultiTypeQueueTest, PushMultipleGetMultiple)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER);
    const MessageType messageType {MessageType::STATELESS};
    const Message messageToSend {messageType, MULTIPLE_DATA_CONTENT};

    EXPECT_EQ(3, multiTypeQueue.push(messageToSend));
    EXPECT_EQ(multiTypeQueue.storedItems(MessageType::STATELESS), 3);
    EXPECT_EQ(multiTypeQueue.popN(MessageType::STATELESS, 1), 1);
    EXPECT_EQ(multiTypeQueue.popN(MessageType::STATELESS, 3), 2);
    EXPECT_TRUE(multiTypeQueue.isEmpty(MessageType::STATELESS));
    EXPECT_EQ(0, multiTypeQueue.storedItems(MessageType::STATELESS));
}

TEST_F(MultiTypeQueueTest, PushAwaitable)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER_SMALL_SIZE);
    boost::asio::io_context io_context;

    int upperLimit = SMALL_QUEUE_CAPACITY + 1;
    for (int i : std::views::iota(1, upperLimit))
    {
        const nlohmann::json dataContent = R"({"Data" : "for STATEFUL)" + std::to_string(i) + R"("})";
        EXPECT_EQ(multiTypeQueue.push({MessageType::STATEFUL, dataContent}), 1);
    }

    EXPECT_TRUE(multiTypeQueue.isFull(MessageType::STATEFUL));
    EXPECT_EQ(multiTypeQueue.storedItems(MessageType::STATEFUL), SMALL_QUEUE_CAPACITY);

    // Coroutine that waits till there's space to push a new message
    boost::asio::co_spawn(
        io_context,
        // NOLINTNEXTLINE(cppcoreguidelines-avoid-capturing-lambda-coroutines)
        [&multiTypeQueue]() -> boost::asio::awaitable<void>
        {
            const nlohmann::json dataContent = {"content-1"};
            const Message messageToSend {MessageType::STATEFUL, dataContent};
            EXPECT_EQ(multiTypeQueue.storedItems(MessageType::STATEFUL), SMALL_QUEUE_CAPACITY);
            auto messagesPushed = co_await multiTypeQueue.pushAwaitable(messageToSend);
            EXPECT_EQ(messagesPushed, 1);
            EXPECT_EQ(multiTypeQueue.storedItems(MessageType::STATEFUL), SMALL_QUEUE_CAPACITY);
        },
        boost::asio::detached);

    // Simulate poping one message so there's space to push a new one
    std::thread consumer(
        [&multiTypeQueue]()
        {
            std::this_thread::sleep_for(std::chrono::seconds(2));
            EXPECT_EQ(multiTypeQueue.popN(MessageType::STATEFUL, 1), 1);
            // TODO: double check this behavior, is it mandatory to stop the context here?
        });

    io_context.run();
    consumer.join();

    EXPECT_TRUE(multiTypeQueue.isFull(MessageType::STATEFUL));
}

TEST_F(MultiTypeQueueTest, FifoOrderCheck)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER);

    // complete the queue with messages
    const MessageType messageType {MessageType::STATEFUL};
    size_t contentSize = 0;
    for (int i : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10})
    {
        const nlohmann::json dataContent = {{"Data", "for STATEFUL" + std::to_string(i)}};
        EXPECT_EQ(multiTypeQueue.push({messageType, dataContent}), 1);
        contentSize += dataContent.dump().size();
    }

    auto messageReceivedVector =
        multiTypeQueue.getNextBytes(messageType, contentSize); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
    EXPECT_EQ(messageReceivedVector.size(), 10);

    std::for_each(messageReceivedVector.begin(),
                  messageReceivedVector.end(),
                  [i = 0](const auto& singleMessage) mutable {
                      EXPECT_EQ(singleMessage.data, (nlohmann::json {{"Data", "for STATEFUL" + std::to_string(++i)}}));
                  });

    // Keep the order of the message: FIFO
    for (int i : {1, 2, 3, 4, 5, 6, 7, 8, 9, 10})
    {
        auto messageReceived = multiTypeQueue.getNextBytes(messageType, 1);
        EXPECT_EQ(messageReceived[0].data, (nlohmann::json {{"Data", "for STATEFUL" + std::to_string(i)}}));
        EXPECT_TRUE(multiTypeQueue.pop(messageType));
    }
}

TEST_F(MultiTypeQueueTest, GetBySizeAboveMax)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER);
    const MessageType messageType {MessageType::STATELESS};
    const std::string moduleName = "testModule";
    const Message messageToSend {messageType, MULTIPLE_DATA_CONTENT, moduleName};

    EXPECT_EQ(3, multiTypeQueue.push(messageToSend));

    // Size request should contemplate data and module name string size
    size_t sizeAsked = 0;
    for (const auto& message : MULTIPLE_DATA_CONTENT)
    {
        sizeAsked += message.dump().size();
        sizeAsked += moduleName.size();
    }
    // Duplicate to surpass the maximun
    sizeAsked *= 2;

    auto messagesReceived = multiTypeQueue.getNextBytes(MessageType::STATELESS, sizeAsked);
    int i = 0;
    for (const auto& singleMessage : messagesReceived)
    {
        EXPECT_EQ("content " + std::to_string(++i), singleMessage.data.get<std::string>());
    }

    EXPECT_EQ(3, multiTypeQueue.storedItems(MessageType::STATELESS, moduleName));
}

TEST_F(MultiTypeQueueTest, GetByBelowMax)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER);
    const MessageType messageType {MessageType::STATELESS};
    const std::string moduleName = "testModule";
    const Message messageToSend {messageType, MULTIPLE_DATA_CONTENT, moduleName};

    EXPECT_EQ(3, multiTypeQueue.push(messageToSend));

    size_t sizeAsked = 0;
    sizeAsked += MULTIPLE_DATA_CONTENT.at(0).dump().size();
    sizeAsked += moduleName.size();
    // Fetching less than a single message size
    sizeAsked -= 1;

    auto messagesReceived = multiTypeQueue.getNextBytes(MessageType::STATELESS, sizeAsked);
    EXPECT_EQ(1, messagesReceived.size());
}
