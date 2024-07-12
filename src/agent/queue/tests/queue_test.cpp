#include <filesystem>

#include "queue.hpp"
#include "queue_test.hpp"

#define BIG_QUEUE_QTTY   10
#define SMALL_QUEUE_QTTY 10

// TODO: redo for additional stores
bool cleanPersistence()
{

    for (auto messageType : MessageTypeName)
    {
        auto filePath = DEFAULT_PERS_PATH + messageType.second;
        std::error_code ec;
        if (std::filesystem::remove(filePath, ec))
        {
            continue;
        }
        else
        {
            std::cerr << "Error removing file: " << ec.message() << std::endl;
            return false;
        }
    }
    return true;
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

    std::cout << "str1: " << str1 << std::endl;
    std::cout << "str2: " << str2 << std::endl;

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

        // Compare the json objects
        return json1 == json2;
    }
    catch (const nlohmann::json::parse_error& e)
    {
        std::cerr << "JSON parsing error: " << e.what() << std::endl;
        return false;
    }
}

void QueueTest::SetUp()
{
    EXPECT_TRUE(cleanPersistence());
};

void QueueTest::TearDown() {};

// Push, get and check the queue is not empty
TEST_F(QueueTest, BasicPushGetNotEmpty)
{
    MultiTypeQueue queue(BIG_QUEUE_QTTY);
    const MessageType messageTtpe {MessageType::STATE_LESS};
    const nlohmann::json dataContent = R"({"Data" : "for STATE_LESS)" + std::to_string(0) + R"("})";
    const Message messageToSend {messageTtpe, dataContent};

    queue.push(messageToSend);
    auto messageResponse = queue.getLastMessage(MessageType::STATE_LESS);

    auto typeSend = messageResponse.type;
    auto typeReceived = messageResponse.type;

    EXPECT_TRUE(typeSend == typeReceived);
    EXPECT_TRUE(compareJsonStrings(messageResponse.data[0], messageToSend.data));
    EXPECT_FALSE(queue.isEmptyByType(MessageType::STATE_LESS));
}

// push and pop on a full queue
TEST_F(QueueTest, BasicPushPopEmpty)
{
    MultiTypeQueue queue(BIG_QUEUE_QTTY);
    const MessageType messageTtpe {MessageType::STATE_LESS};
    const nlohmann::json dataContent = R"({"Data" : "for STATE_LESS)" + std::to_string(0) + R"("})";
    const Message messageToSend {messageTtpe, dataContent};

    queue.push(messageToSend);
    auto messageResponse = queue.getLastMessage(MessageType::STATE_LESS);

    auto typeSend = messageResponse.type;
    auto typeReceived = messageResponse.type;

    EXPECT_TRUE(typeSend == typeReceived);
    EXPECT_TRUE(compareJsonStrings(messageResponse.data[0], messageToSend.data));
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATE_LESS));
}

// Push, pop and check the queue is empty
TEST_F(QueueTest, BasicPushPop)
{
    MultiTypeQueue queue(BIG_QUEUE_QTTY);
    const MessageType messageTtpe {MessageType::STATE_LESS};
    const nlohmann::json dataContent = R"({"Data" : "for STATE_LESS)" + std::to_string(0) + R"("})";
    const Message messageToSend {messageTtpe, dataContent};

    queue.push(messageToSend);
    EXPECT_FALSE(queue.isEmptyByType(MessageType::STATE_LESS));

    queue.popLastMessage(MessageType::STATE_LESS);
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATE_LESS));
}

// get and pop on a empty queue
TEST_F(QueueTest, BasicGetPopOnEmpty)
{
    MultiTypeQueue queue(BIG_QUEUE_QTTY);
    const MessageType messageTtpe {MessageType::STATE_LESS};
    const nlohmann::json dataContent = R"({"Data" : "for STATE_LESS)" + std::to_string(0) + R"("})";
    const Message messageToSend {messageTtpe, dataContent};

    queue.push(messageToSend);
    EXPECT_FALSE(queue.isEmptyByType(MessageType::STATE_LESS));
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATE_FULL));

    // TODO: double check
    const nlohmann::json emptyContent = R"({})";
    auto messageResponse = queue.getLastMessage(MessageType::STATE_FULL);
    EXPECT_TRUE(messageResponse.data == emptyContent);
    EXPECT_EQ(messageResponse.type, MessageType::STATE_FULL);

    queue.popLastMessage(MessageType::STATE_FULL);
    EXPECT_TRUE(queue.isEmptyByType(MessageType::STATE_FULL));
}

// TODO
TEST_F(QueueTest, Multithread)
{
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