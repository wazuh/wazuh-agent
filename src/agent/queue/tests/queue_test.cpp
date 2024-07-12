
#include <iomanip>

#include <chrono>
#include <filesystem>
#include <future>
#include <iostream>
#include <stdexcept>
#include <thread>

#include "queue.hpp"
#include "queue_test.hpp"

#define BIG_QUEUE_QTTY   10
#define SMALL_QUEUE_QTTY 2

/// Helper functions

// TODO: Makes sense to add a full_clean method inside the persistence implementation?
void cleanPersistence()
{
    auto filePath = DEFAULT_DB_PATH;
    std::error_code ec;
    std::filesystem::remove(filePath, ec);
}

// TODO: rework after double check the output of persistence
bool compareJsonStrings(const nlohmann::json& jsonValue1, const nlohmann::json& jsonValue2)
{

    std::string str1 = jsonValue1.get<std::string>();
    std::string str2 = jsonValue2.get<std::string>();
    // Remove outer quotes from output if present
    auto removeQuotes = [](std::string& inputString)
    {
        if (inputString.front() == '"' && inputString.back() == '"')
        {
            inputString = inputString.substr(1, inputString.length() - 2);
        }
    };

    removeQuotes(str1);
    removeQuotes(str2);

    auto unescapeJsonString = [](std::string input) -> std::string
    {
        std::string result;
        bool escapeNext = false;
        for (char c : input)
        {
            if (escapeNext)
            {
                if (c == '"')
                {
                    result += '"';
                }
                else
                {
                    result += '\\';
                    result += c;
                }
                escapeNext = false;
            }
            else
            {
                if (c == '\\')
                {
                    escapeNext = true;
                }
                else
                {
                    result += c;
                }
            }
        }
        return result;
    };

    unescapeJsonString(str1);
    unescapeJsonString(str2);

    try
    {
        // Parse both strings into json objects
        nlohmann::json json1 = nlohmann::json::parse(str1);
        nlohmann::json json2 = nlohmann::json::parse(str2);

        std::cout << "json1: " << json1 << std::endl;
        std::cout << "json2: " << json2 << std::endl;

        // Compare the json objects
        return (json1 == json2);
    }
    catch (const nlohmann::json::parse_error& e)
    {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return false;
    }
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

// Check which jsons are correct, do move or delete if out of scope
TEST_F(QueueTest, JSONBaseTest)
{
    nlohmann::json uj1 = {{"version", 1}, {"type", "integer"}};
    nlohmann::json uj2 = {{"type", "integer"}, {"version", 1}};

    nlohmann::ordered_json oj1 = {{"version", 1}, {"type", "integer"}};
    nlohmann::ordered_json oj2 = {{"type", "integer"}, {"version", 1}};

    EXPECT_TRUE(uj1 == uj2);
    //EXPECT_TRUE(compareJsonStrings(uj1, uj2));
    EXPECT_FALSE(oj1 == oj2);
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
    // EXPECT_TRUE(compareJsonStrings(messageResponse.data[0], messageToSend.data));
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
    // EXPECT_TRUE(compareJsonStrings(messageResponse.data[0], messageToSend.data));

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
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATE_FULL));

    auto messageResponse = queue.getLastMessage(MessageType::STATE_FULL);
    EXPECT_EQ(messageResponse.type, MessageType::STATE_FULL);
    EXPECT_EQ(messageResponse.data[0], nullptr);

    queue.popLastMessage(MessageType::STATE_FULL);
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATE_FULL));
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
        functionWithTimeout(
            [&queue](Message& message) { queue.push(message); }, 1000, exampleMessage);
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

// TODO
TEST_F(QueueTest, Multithread)
{
    GTEST_SKIP();
    MultiTypeQueue queue(BIG_QUEUE_QTTY);

    auto consumer1 = [&](int& count)
    {
        // std::this_thread::sleep_for(std::chrono::milliseconds(1000));
        int item;
        for (int i = 0; i < count; ++i)
        {
            queue.popLastMessage(MessageType::STATE_LESS);
            std::cout << "Popping event 1: " << std::endl;
        }
    };

    auto consumer2 = [&](int& count)
    {
        int item;
        for (int i = 0; i < count; ++i)
        {
            queue.popLastMessage(MessageType::STATE_FULL);
            std::cout << "Popping event 2: " << std::endl;
        }
    };

    auto producer = [&](int& count)
    {
        for (int i = 0; i < count; ++i)
        {
            auto message = R"({"Data" : "for STATE_LESS)" + std::to_string(i) + R"("})";
            queue.push(Message(MessageType::STATE_LESS, message));
            std::this_thread::sleep_for(std::chrono::milliseconds(100)); // Simulate work
        }
    };

    int items_to_insert = 5;
    int items_to_consume = 2;

    producer(items_to_insert);

    std::this_thread::sleep_for(std::chrono::milliseconds(1000));

    std::thread consumer_thread1(consumer1, std::ref(items_to_consume));
    std::thread producer_thread1(consumer2, std::ref(items_to_consume));

    producer_thread1.join();
    consumer_thread1.join();

    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATE_LESS));
}
