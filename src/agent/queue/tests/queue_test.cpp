#include <chrono>
#include <filesystem>
#include <future>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <stdexcept>
#include <thread>

#include "queue.hpp"
#include "queue_test.hpp"

#define BIG_QUEUE_QTTY   10
#define SMALL_QUEUE_QTTY 2

using json = nlohmann::json;

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

// In order to mimic the timeout from outside the queue
// TODO: double check if this is expected to work this way
template<typename Func, typename... Args>
void functionWithTimeout(Func&& func, int timeout_ms, Args&&... args)
{
    // Launch the function in a separate thread
    std::promise<void> exitSignal;
    std::future<void> futureObj = exitSignal.get_future();

    std::thread t(
        [&func, &exitSignal, &args...]()
        {
            func(std::forward<Args>(args)...);
            exitSignal.set_value();
        });

    // Wait for the function to finish or timeout
    if (futureObj.wait_for(std::chrono::milliseconds(timeout_ms)) == std::future_status::timeout)
    {
        // Detach the thread if it times out
        t.detach();
        throw std::runtime_error("Function call timed out");
    }
    else
    {
        t.join();
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
TEST_F(QueueTest, JSONConversionComparisson)
{
    nlohmann::json uj1 = {{"version", 1}, {"type", "integer"}};
    // From string. If not unescape then it throws errors
    nlohmann::json uj2 = json::parse(unescape_string(R"({\"type\":\"integer\",\"version\":1})"));

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

// Push, get and check the queue is not empty
TEST_F(QueueTest, BasicPushGetNotEmpty)
{
    MultiTypeQueue queue(BIG_QUEUE_QTTY);
    const MessageType messageType {MessageType::STATE_LESS};
    const nlohmann::json dataContent = R"({{"Data", "for STATE_LESS)" + std::to_string(0) + R"("}})";
    const Message messageToSend {messageType, dataContent};

    queue.push(messageToSend);
    auto messageResponse = queue.getLastMessage(MessageType::STATE_LESS);

    auto typeSend = messageResponse.type;
    auto typeReceived = messageResponse.type;

    EXPECT_TRUE(typeSend == typeReceived);
    auto versionUj1 = messageResponse.data[0].template get<std::string>();
    auto versionUj2 = messageToSend.data.template get<std::string>();
    EXPECT_EQ(versionUj1, versionUj2);

    EXPECT_FALSE(queue.isEmptyByType(MessageType::STATE_LESS));
}

// push and pop on a non-full queue
TEST_F(QueueTest, SinglePushPopEmpty)
{
    MultiTypeQueue queue(BIG_QUEUE_QTTY);
    const MessageType messageType {MessageType::STATE_LESS};
    const nlohmann::json dataContent = R"({{"Data", "for STATE_LESS)" + std::to_string(0) + R"("}})";
    const Message messageToSend {messageType, dataContent};

    queue.push(messageToSend);
    auto messageResponse = queue.getLastMessage(MessageType::STATE_LESS);

    auto typeSend = messageResponse.type;
    auto typeReceived = messageResponse.type;

    EXPECT_TRUE(typeSend == typeReceived);
    auto versionUj1 = messageResponse.data[0].template get<std::string>();
    auto versionUj2 = messageToSend.data.template get<std::string>();
    EXPECT_EQ(versionUj1, versionUj2);

    queue.popLastMessage(MessageType::STATE_LESS);
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATE_LESS));
}

// Push, pop and check the queue is empty
TEST_F(QueueTest, SinglePushPop)
{
    MultiTypeQueue queue(BIG_QUEUE_QTTY);
    const MessageType messageType {MessageType::STATE_LESS};
    const nlohmann::json dataContent = R"({"Data" : "for STATE_LESS)" + std::to_string(0) + R"("})";
    const Message messageToSend {messageType, dataContent};

    queue.push(messageToSend);
    EXPECT_FALSE(queue.isEmptyByType(MessageType::STATE_LESS));

    queue.popLastMessage(MessageType::STATE_LESS);
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATE_LESS));
}

// get and pop on a empty queue
TEST_F(QueueTest, SingleGetPopOnEmpty)
{
    MultiTypeQueue queue(BIG_QUEUE_QTTY);
    const MessageType messageType {MessageType::STATE_LESS};
    const nlohmann::json dataContent = R"({"Data" : "for STATE_LESS)" + std::to_string(0) + R"("})";
    const Message messageToSend {messageType, dataContent};

    queue.push(messageToSend);
    EXPECT_FALSE(queue.isEmptyByType(MessageType::STATE_LESS));
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATE_FUL));

    auto messageResponse = queue.getLastMessage(MessageType::STATE_FUL);
    EXPECT_EQ(messageResponse.type, MessageType::STATE_FUL);
    EXPECT_EQ(messageResponse.data[0], nullptr);

    queue.popLastMessage(MessageType::STATE_FUL);
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATE_FUL));
}

// Push, get and check while the queue is full
TEST_F(QueueTest, SinglePushPopFull)
{
    MultiTypeQueue queue(SMALL_QUEUE_QTTY);

    // complete the queue with messages
    const MessageType messageType {MessageType::COMMAND};
    for (int i : {1, 2})
    {
        const nlohmann::json dataContent = R"({"Data" : "for COMMAND)" + std::to_string(i) + R"("})";
        queue.push({messageType, dataContent});
    }

    const nlohmann::json dataContent = R"({"Data" : "for COMMAND3"})";
    Message exampleMessage {messageType, dataContent};

    try
    {
        functionWithTimeout([&queue](Message& message) { queue.push(message); }, 1000, exampleMessage);
    }
    catch (const std::runtime_error& e)
    {
        std::cerr << e.what() << '\n';
    }

    auto items = queue.getItemsByType(MessageType::COMMAND);
    EXPECT_EQ(items, SMALL_QUEUE_QTTY);
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATE_LESS));

    queue.popLastMessage(MessageType::COMMAND);
    items = queue.getItemsByType(MessageType::COMMAND);
    EXPECT_NE(items, SMALL_QUEUE_QTTY);
}

// Accesing different types of queues
TEST_F(QueueTest, MultithreadDifferentType)
{
    MultiTypeQueue queue(BIG_QUEUE_QTTY);

    auto consumerStateLess = [&](int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            queue.popLastMessage(MessageType::STATE_LESS);
        }
    };

    auto consumerStateFull = [&](int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            queue.popLastMessage(MessageType::STATE_FUL);
        }
    };

    auto messageProducer = [&](int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            const nlohmann::json dataContent = R"({{"Data", "for STATE_LESS)" + std::to_string(i) + R"("}})";
            queue.push(Message(MessageType::STATE_LESS, dataContent));
            queue.push(Message(MessageType::STATE_FUL, dataContent));
        }
    };

    int itemsToInsert = 9;
    int itemsToConsume = 5;

    messageProducer(itemsToInsert);

    std::thread consumerThread1(consumerStateLess, std::ref(itemsToConsume));
    std::thread consumerThread2(consumerStateFull, std::ref(itemsToConsume));

    consumerThread1.join();
    consumerThread2.join();

    EXPECT_FALSE(queue.isEmptyByType(MessageType::STATE_LESS));
    EXPECT_FALSE(queue.isEmptyByType(MessageType::STATE_FUL));

    std::thread consumerThread12(consumerStateLess, std::ref(itemsToConsume));
    std::thread consumerThread22(consumerStateFull, std::ref(itemsToConsume));

    consumerThread22.join();
    consumerThread12.join();

    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATE_LESS));
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATE_FUL));
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
            const nlohmann::json dataContent = R"({{"Data", "for COMMAND)" + std::to_string(i) + R"("}})";
            queue.push(Message(MessageType::COMMAND, dataContent));
        }
    };

    int itemsToInsert = 10;
    int itemsToConsume = 5;

    messageProducer(itemsToInsert);

    EXPECT_EQ(itemsToInsert, queue.getItemsByType(MessageType::COMMAND));

    std::thread consumerThread1(consumerCommand1, std::ref(itemsToConsume));
    std::thread messageProducerThread1(consumerCommand2, std::ref(itemsToConsume));

    messageProducerThread1.join();
    consumerThread1.join();

    EXPECT_TRUE(queue.isEmptyByType(MessageType::COMMAND));
}
