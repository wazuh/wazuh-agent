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

#include <mock_storage.hpp>
#include <multitype_queue.hpp>

#include "multitype_queue_test.hpp"

const nlohmann::json BASE_DATA_CONTENT = R"({{"data": "for STATELESS_0"}})";
const nlohmann::json MULTIPLE_DATA_CONTENT = {"content 1", "content 2", "content 3"};
const int DEFAULT_QUEUE_SIZE = 10000;

// NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines,cppcoreguidelines-avoid-reference-coroutine-parameters)

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

    const auto MOCK_CONFIG_PARSER = std::make_shared<configuration::ConfigurationParser>(std::string(R"(
        agent:
          path.data: "."
    )"));
} // namespace

/// Test Methods

void MultiTypeQueueTest::SetUp()
{
    m_mockStoragePtr = std::make_unique<MockStorage>();
    m_mockStorage = m_mockStoragePtr.get();
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

    auto mockStorage = std::make_unique<MockStorage>();

    EXPECT_NO_THROW(const MultiTypeQueue multiTypeQueue(configurationParser, std::move(mockStorage)));
}

TEST_F(MultiTypeQueueTest, ConstructorNoConfigParser)
{
    auto mockStorage = std::make_unique<MockStorage>();

    EXPECT_THROW(const MultiTypeQueue multiTypeQueue(nullptr, std::move(mockStorage)), std::runtime_error);
}

TEST_F(MultiTypeQueueTest, PushGetNotQueue)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {static_cast<MessageType>(10)};
    const Message messageToSend {messageType, BASE_DATA_CONTENT};

    EXPECT_EQ(multiTypeQueue.push(messageToSend), 0);
}

TEST_F(MultiTypeQueueTest, PushNotSpaceAvailable)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};
    const Message messageToSend {messageType, BASE_DATA_CONTENT};

    EXPECT_CALL(*m_mockStorage, GetElementCount(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(DEFAULT_QUEUE_SIZE));

    EXPECT_EQ(multiTypeQueue.push(messageToSend), 0);
}

TEST_F(MultiTypeQueueTest, PushStoreMessage)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};
    const nlohmann::json sigleData = R"({"data": "for STATELESS_0"})";
    const Message messageToSend {messageType, sigleData};

    EXPECT_CALL(*m_mockStorage, GetElementCount(testing::_, testing::_, testing::_)).WillOnce(testing::Return(0));

    EXPECT_CALL(*m_mockStorage, Store(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(1));

    EXPECT_EQ(multiTypeQueue.push(messageToSend), 1);
}

TEST_F(MultiTypeQueueTest, PushStoreArray)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};
    nlohmann::json arrayData = nlohmann::json::array();
    arrayData.push_back({{"data1", "for STATELESS_0"}});
    arrayData.push_back({{"data2", "for STATELESS_0"}});

    const Message messageToSend {messageType, arrayData};

    EXPECT_CALL(*m_mockStorage, GetElementCount(testing::_, testing::_, testing::_)).WillOnce(testing::Return(0));

    EXPECT_CALL(*m_mockStorage, Store(testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(2)
        .WillRepeatedly(testing::Return(1));

    EXPECT_EQ(multiTypeQueue.push(messageToSend), 2);
}

TEST_F(MultiTypeQueueTest, PushStoreArrayFailFirst)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};
    nlohmann::json arrayData = nlohmann::json::array();
    arrayData.push_back({{"data1", "for STATELESS_0"}});
    arrayData.push_back({{"data2", "for STATELESS_0"}});

    const Message messageToSend {messageType, arrayData};

    EXPECT_CALL(*m_mockStorage, GetElementCount(testing::_, testing::_, testing::_)).WillOnce(testing::Return(0));

    const testing::Sequence seq;
    EXPECT_CALL(*m_mockStorage, Store(testing::_, testing::_, testing::_, testing::_, testing::_))
        .InSequence(seq)
        .WillOnce(testing::Return(0))
        .WillOnce(testing::Return(1));

    EXPECT_EQ(multiTypeQueue.push(messageToSend), 1);
}

TEST_F(MultiTypeQueueTest, PushStoreArrayNotSpaceAvailable)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};
    nlohmann::json arrayData = nlohmann::json::array();
    arrayData.push_back({{"data1", "for STATELESS_0"}});
    arrayData.push_back({{"data2", "for STATELESS_0"}});

    const Message messageToSend {messageType, arrayData};

    EXPECT_CALL(*m_mockStorage, GetElementCount(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(DEFAULT_QUEUE_SIZE - 1));

    EXPECT_EQ(multiTypeQueue.push(messageToSend), 0);
}

TEST_F(MultiTypeQueueTest, PushAwaitableNotQueue)
{
    boost::asio::io_context ioContext;
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));

    const MessageType messageType {static_cast<MessageType>(10)};
    const Message messageToSend {messageType, BASE_DATA_CONTENT};

    testing::MockFunction<void(int)> checkResult;
    EXPECT_CALL(checkResult, Call(0));

    boost::asio::co_spawn(
        ioContext,
        [&]() -> boost::asio::awaitable<void>
        {
            const int result = co_await multiTypeQueue.pushAwaitable(messageToSend);
            checkResult.Call(result);
        },
        boost::asio::detached);

    ioContext.run();
}

TEST_F(MultiTypeQueueTest, PushAwaitableNotSpaceAvailable)
{
    boost::asio::io_context ioContext;
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));

    const MessageType messageType {MessageType::STATELESS};
    const Message messageToSend {messageType, BASE_DATA_CONTENT};

    const testing::Sequence seq;
    EXPECT_CALL(*m_mockStorage, GetElementCount(testing::_, testing::_, testing::_))
        .InSequence(seq)
        .WillOnce(testing::Return(DEFAULT_QUEUE_SIZE - 1))
        .WillOnce(testing::Return(DEFAULT_QUEUE_SIZE));

    testing::MockFunction<void(int)> checkResult;
    EXPECT_CALL(checkResult, Call(0));

    boost::asio::co_spawn(
        ioContext,
        [&]() -> boost::asio::awaitable<void>
        {
            const int result = co_await multiTypeQueue.pushAwaitable(messageToSend);
            checkResult.Call(result);
        },
        boost::asio::detached);

    ioContext.run();
}

TEST_F(MultiTypeQueueTest, PushAwaitableStoreMessage)
{
    boost::asio::io_context ioContext;
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};
    const nlohmann::json sigleData = R"({"data": "for STATELESS_0"})";
    const Message messageToSend {messageType, sigleData};

    EXPECT_CALL(*m_mockStorage, GetElementCount(testing::_, testing::_, testing::_))
        .Times(2)
        .WillRepeatedly(testing::Return(0));

    EXPECT_CALL(*m_mockStorage, Store(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(1));

    testing::MockFunction<void(int)> checkResult;
    EXPECT_CALL(checkResult, Call(1));

    boost::asio::co_spawn(
        ioContext,
        [&]() -> boost::asio::awaitable<void>
        {
            const int result = co_await multiTypeQueue.pushAwaitable(messageToSend);
            checkResult.Call(result);
        },
        boost::asio::detached);

    ioContext.run();
}

TEST_F(MultiTypeQueueTest, PushAwaitableStoreArray)
{
    boost::asio::io_context ioContext;
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};
    nlohmann::json arrayData = nlohmann::json::array();
    arrayData.push_back({{"data1", "for STATELESS_0"}});
    arrayData.push_back({{"data2", "for STATELESS_0"}});

    const Message messageToSend {messageType, arrayData};

    EXPECT_CALL(*m_mockStorage, GetElementCount(testing::_, testing::_, testing::_))
        .Times(2)
        .WillRepeatedly(testing::Return(0));

    EXPECT_CALL(*m_mockStorage, Store(testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(2)
        .WillRepeatedly(testing::Return(1));

    testing::MockFunction<void(int)> checkResult;
    EXPECT_CALL(checkResult, Call(2));

    boost::asio::co_spawn(
        ioContext,
        [&]() -> boost::asio::awaitable<void>
        {
            const int result = co_await multiTypeQueue.pushAwaitable(messageToSend);
            checkResult.Call(result);
        },
        boost::asio::detached);

    ioContext.run();
}

TEST_F(MultiTypeQueueTest, PushAwaitableStoreArrayFailFirst)
{
    boost::asio::io_context ioContext;
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};
    nlohmann::json arrayData = nlohmann::json::array();
    arrayData.push_back({{"data1", "for STATELESS_0"}});
    arrayData.push_back({{"data2", "for STATELESS_0"}});

    const Message messageToSend {messageType, arrayData};

    EXPECT_CALL(*m_mockStorage, GetElementCount(testing::_, testing::_, testing::_))
        .Times(2)
        .WillRepeatedly(testing::Return(0));

    const testing::Sequence seq;
    EXPECT_CALL(*m_mockStorage, Store(testing::_, testing::_, testing::_, testing::_, testing::_))
        .InSequence(seq)
        .WillOnce(testing::Return(0))
        .WillOnce(testing::Return(1));

    testing::MockFunction<void(int)> checkResult;
    EXPECT_CALL(checkResult, Call(1));

    boost::asio::co_spawn(
        ioContext,
        [&]() -> boost::asio::awaitable<void>
        {
            const int result = co_await multiTypeQueue.pushAwaitable(messageToSend);
            checkResult.Call(result);
        },
        boost::asio::detached);

    ioContext.run();
}

TEST_F(MultiTypeQueueTest, PushAwaitableStoreArrayNotSpaceAvailable)
{
    boost::asio::io_context ioContext;
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};
    nlohmann::json arrayData = nlohmann::json::array();
    arrayData.push_back({{"data1", "for STATELESS_0"}});
    arrayData.push_back({{"data2", "for STATELESS_0"}});

    const Message messageToSend {messageType, arrayData};

    EXPECT_CALL(*m_mockStorage, GetElementCount(testing::_, testing::_, testing::_))
        .Times(2)
        .WillRepeatedly(testing::Return(DEFAULT_QUEUE_SIZE - 1));

    testing::MockFunction<void(int)> checkResult;
    EXPECT_CALL(checkResult, Call(0));

    boost::asio::co_spawn(
        ioContext,
        [&]() -> boost::asio::awaitable<void>
        {
            const int result = co_await multiTypeQueue.pushAwaitable(messageToSend);
            checkResult.Call(result);
        },
        boost::asio::detached);

    ioContext.run();
}

TEST_F(MultiTypeQueueTest, PushVector)
{
    std::vector<Message> messages = {};
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};
    nlohmann::json arrayData = nlohmann::json::array();
    arrayData.push_back({{"data1", "for STATELESS_0"}});
    arrayData.push_back({{"data2", "for STATELESS_0"}});
    const Message messageToSend {messageType, arrayData};

    nlohmann::json arrayData2 = nlohmann::json::array();
    arrayData2.push_back({{"data3", "for STATELESS_0"}});
    arrayData2.push_back({{"data4", "for STATELESS_0"}});
    const Message messageToSend2 {messageType, arrayData2};

    messages.push_back(messageToSend);
    messages.push_back(messageToSend2);

    EXPECT_CALL(*m_mockStorage, GetElementCount(testing::_, testing::_, testing::_))
        .Times(2)
        .WillRepeatedly(testing::Return(0));

    EXPECT_CALL(*m_mockStorage, Store(testing::_, testing::_, testing::_, testing::_, testing::_))
        .Times(4)
        .WillRepeatedly(testing::Return(1));

    EXPECT_EQ(multiTypeQueue.push(messages), 4);
}

TEST_F(MultiTypeQueueTest, GetNextBadQueue)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {static_cast<MessageType>(10)};

    auto message = multiTypeQueue.getNext(messageType);

    EXPECT_EQ(message.type, messageType);
    EXPECT_EQ(message.data, "{}"_json);
}

TEST_F(MultiTypeQueueTest, GetNextEmpty)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));

    const nlohmann::json retrieveResult = nlohmann::json::array();
    EXPECT_CALL(*m_mockStorage, RetrieveMultiple(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(retrieveResult));

    auto message = multiTypeQueue.getNext(MessageType::STATELESS);

    EXPECT_EQ(message.type, MessageType::STATELESS);
    EXPECT_EQ(message.data, "{}"_json);
}

TEST_F(MultiTypeQueueTest, GetNextMessage)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));

    const std::string moduleName = "TestModule";
    const std::string moduleType = "TestType";

    nlohmann::json retrieveResult = nlohmann::json::array();
    const nlohmann::json outputJson = {{"moduleName", "TestModule"},
                                       {"moduleType", "TestType"},
                                       {"metadata", "TestMetadata"},
                                       {"data", BASE_DATA_CONTENT}};
    retrieveResult.push_back(outputJson);
    EXPECT_CALL(*m_mockStorage, RetrieveMultiple(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(retrieveResult));

    auto message = multiTypeQueue.getNext(MessageType::STATELESS, moduleName, moduleType);

    EXPECT_EQ(message.type, MessageType::STATELESS);
    EXPECT_EQ(message.data, BASE_DATA_CONTENT);
    EXPECT_EQ(message.moduleName, moduleName);
    EXPECT_EQ(message.moduleType, moduleType);
}

TEST_F(MultiTypeQueueTest, GetNextBytesAwaitableBadQueue)
{
    boost::asio::io_context ioContext;
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));

    const MessageType messageType {static_cast<MessageType>(10)};
    const size_t messageQuantity = 5;
    const std::string moduleName = "TestModule";
    const std::string moduleType = "TestType";

    testing::MockFunction<void(std::vector<Message>)> checkResult;
    EXPECT_CALL(checkResult, Call(testing::IsEmpty()));

    boost::asio::co_spawn(
        ioContext,
        [&]() -> boost::asio::awaitable<void>
        {
            auto result =
                co_await multiTypeQueue.getNextBytesAwaitable(messageType, messageQuantity, moduleName, moduleType);
            checkResult.Call(result);
        },
        boost::asio::detached);

    ioContext.run();
}

TEST_F(MultiTypeQueueTest, GetNextBytesAwaitableSuccess)
{
    boost::asio::io_context ioContext;
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));

    const MessageType messageType {MessageType::STATELESS};
    const size_t messageQuantity = 3;

    const nlohmann::json retrievedMessages = nlohmann::json::array(
        {{{"data", "msg1"}, {"moduleName", "mod1"}, {"moduleType", "type1"}, {"metadata", "meta1"}},
         {{"data", "msg2"}, {"moduleName", "mod2"}, {"moduleType", "type2"}, {"metadata", "meta2"}},
         {{"data", "msg3"}, {"moduleName", "mod3"}, {"moduleType", "type3"}, {"metadata", "meta3"}}});

    const nlohmann::json msgData1 = "msg1";
    const nlohmann::json msgData2 = "msg2";
    const nlohmann::json msgData3 = "msg3";

    const std::vector<Message> expectedMessages = {{messageType, msgData1, "mod1", "type1", "meta1"},
                                                   {messageType, msgData2, "mod2", "type2", "meta2"},
                                                   {messageType, msgData3, "mod3", "type3", "meta3"}};

    EXPECT_CALL(*m_mockStorage, GetElementsStoredSize(testing::_, testing::_, testing::_))
        .WillRepeatedly(testing::Return(messageQuantity));

    EXPECT_CALL(*m_mockStorage, RetrieveBySize(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(retrievedMessages));

    testing::MockFunction<void(const std::vector<Message>&)> checkResult;
    EXPECT_CALL(checkResult, Call(testing::ElementsAreArray(expectedMessages)));

    boost::asio::co_spawn(
        ioContext,
        [&]() -> boost::asio::awaitable<void>
        {
            auto result = co_await multiTypeQueue.getNextBytesAwaitable(messageType, messageQuantity);
            checkResult.Call(result);
        },
        boost::asio::detached);

    ioContext.run();
}

TEST_F(MultiTypeQueueTest, GetNextBytesBadQueue)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {static_cast<MessageType>(10)};

    const size_t contentSize = 1;
    auto messages = multiTypeQueue.getNextBytes(messageType, contentSize);

    EXPECT_EQ(messages.size(), 0);
}

TEST_F(MultiTypeQueueTest, GetNextBytesEmpty)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};

    const nlohmann::json retrieveResult = nlohmann::json::array();
    EXPECT_CALL(*m_mockStorage, RetrieveBySize(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(retrieveResult));

    const size_t contentSize = 1;
    auto messages = multiTypeQueue.getNextBytes(messageType, contentSize);

    EXPECT_EQ(messages.size(), 0);
}

TEST_F(MultiTypeQueueTest, GetNextBytesMessage)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};
    const std::string moduleName = "TestModule";
    const std::string moduleType = "TestType";

    const nlohmann::json retrievedMessages = nlohmann::json::array(
        {{{"data", "msg1"}, {"moduleName", moduleName}, {"moduleType", moduleType}, {"metadata", "meta1"}},
         {{"data", "msg2"}, {"moduleName", moduleName}, {"moduleType", moduleType}, {"metadata", "meta2"}},
         {{"data", "msg3"}, {"moduleName", moduleName}, {"moduleType", moduleType}, {"metadata", "meta3"}}});

    EXPECT_CALL(*m_mockStorage, RetrieveBySize(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(retrievedMessages));

    const size_t contentSize = 3;
    auto messages = multiTypeQueue.getNextBytes(messageType, contentSize, moduleName, moduleType);

    EXPECT_EQ(messages.size(), 3);
    EXPECT_EQ(messages[0].type, MessageType::STATELESS);
    EXPECT_EQ(messages[0].data, "msg1");
    EXPECT_EQ(messages[0].moduleName, moduleName);
    EXPECT_EQ(messages[0].moduleType, moduleType);
    EXPECT_EQ(messages[1].type, MessageType::STATELESS);
    EXPECT_EQ(messages[1].data, "msg2");
    EXPECT_EQ(messages[1].moduleName, moduleName);
    EXPECT_EQ(messages[1].moduleType, moduleType);
    EXPECT_EQ(messages[2].type, MessageType::STATELESS);
    EXPECT_EQ(messages[2].data, "msg3");
    EXPECT_EQ(messages[2].moduleName, moduleName);
    EXPECT_EQ(messages[2].moduleType, moduleType);
}

TEST_F(MultiTypeQueueTest, PopBadQueue)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {static_cast<MessageType>(10)};

    EXPECT_FALSE(multiTypeQueue.pop(messageType));
}

TEST_F(MultiTypeQueueTest, PopEmpty)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};

    EXPECT_CALL(*m_mockStorage, RemoveMultiple(1, testing::_, testing::_, testing::_)).WillOnce(testing::Return(0));

    EXPECT_FALSE(multiTypeQueue.pop(messageType));
}

TEST_F(MultiTypeQueueTest, PopSuccess)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};

    EXPECT_CALL(*m_mockStorage, RemoveMultiple(1, testing::_, testing::_, testing::_)).WillOnce(testing::Return(1));

    EXPECT_TRUE(multiTypeQueue.pop(messageType));
}

TEST_F(MultiTypeQueueTest, PopNBadQueue)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {static_cast<MessageType>(10)};

    const int messageQuantity = 3;
    EXPECT_EQ(multiTypeQueue.popN(messageType, messageQuantity), 0);
}

TEST_F(MultiTypeQueueTest, PopNEmpty)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};

    const int messageQuantity = 3;
    EXPECT_CALL(*m_mockStorage, RemoveMultiple(messageQuantity, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(0));

    EXPECT_EQ(multiTypeQueue.popN(messageType, messageQuantity), 0);
}

TEST_F(MultiTypeQueueTest, PopNSuccess)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};

    const int messageQuantity = 3;
    EXPECT_CALL(*m_mockStorage, RemoveMultiple(messageQuantity, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(messageQuantity));

    EXPECT_EQ(multiTypeQueue.popN(messageType, messageQuantity), messageQuantity);
}

TEST_F(MultiTypeQueueTest, IsEmptyBadQueue)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {static_cast<MessageType>(10)};

    EXPECT_FALSE(multiTypeQueue.isEmpty(messageType));
}

TEST_F(MultiTypeQueueTest, IsEmptyTrue)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};

    EXPECT_CALL(*m_mockStorage, GetElementCount(testing::_, testing::_, testing::_)).WillOnce(testing::Return(0));

    EXPECT_TRUE(multiTypeQueue.isEmpty(messageType));
}

TEST_F(MultiTypeQueueTest, IsEmptyFalse)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};

    EXPECT_CALL(*m_mockStorage, GetElementCount(testing::_, testing::_, testing::_)).WillOnce(testing::Return(2));

    EXPECT_FALSE(multiTypeQueue.isEmpty(messageType));
}

TEST_F(MultiTypeQueueTest, IsFullBadQueue)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {static_cast<MessageType>(10)};

    EXPECT_FALSE(multiTypeQueue.isFull(messageType));
}

TEST_F(MultiTypeQueueTest, IsFullTrue)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};

    EXPECT_CALL(*m_mockStorage, GetElementCount(testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(DEFAULT_QUEUE_SIZE));

    EXPECT_TRUE(multiTypeQueue.isFull(messageType));
}

TEST_F(MultiTypeQueueTest, IsFullFalse)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};

    EXPECT_CALL(*m_mockStorage, GetElementCount(testing::_, testing::_, testing::_)).WillOnce(testing::Return(2));

    EXPECT_FALSE(multiTypeQueue.isFull(messageType));
}

TEST_F(MultiTypeQueueTest, StoredItemsBadQueue)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {static_cast<MessageType>(10)};

    EXPECT_EQ(multiTypeQueue.storedItems(messageType), 0);
}

TEST_F(MultiTypeQueueTest, StoredItemsEmpty)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};

    EXPECT_CALL(*m_mockStorage, GetElementCount(testing::_, testing::_, testing::_)).WillOnce(testing::Return(0));

    EXPECT_EQ(multiTypeQueue.storedItems(messageType), 0);
}

TEST_F(MultiTypeQueueTest, StoredItemsNotEmpty)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};

    EXPECT_CALL(*m_mockStorage, GetElementCount(testing::_, testing::_, testing::_)).WillOnce(testing::Return(2));

    EXPECT_EQ(multiTypeQueue.storedItems(messageType), 2);
}

TEST_F(MultiTypeQueueTest, SizePerTypeBadQueue)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {static_cast<MessageType>(10)};

    EXPECT_EQ(multiTypeQueue.sizePerType(messageType), 0);
}

TEST_F(MultiTypeQueueTest, SizePerTypeEmpty)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};

    EXPECT_CALL(*m_mockStorage, GetElementsStoredSize(testing::_, testing::_, testing::_)).WillOnce(testing::Return(0));

    EXPECT_EQ(multiTypeQueue.sizePerType(messageType), 0);
}

TEST_F(MultiTypeQueueTest, SizePerTypeNotEmpty)
{
    MultiTypeQueue multiTypeQueue(MOCK_CONFIG_PARSER, std::move(m_mockStoragePtr));
    const MessageType messageType {MessageType::STATELESS};

    EXPECT_CALL(*m_mockStorage, GetElementsStoredSize(testing::_, testing::_, testing::_)).WillOnce(testing::Return(2));

    EXPECT_EQ(multiTypeQueue.sizePerType(messageType), 2);
}

// NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines,cppcoreguidelines-avoid-reference-coroutine-parameters)
