#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <communicator.hpp>
#include <ihttp_client.hpp>

#include <jwt-cpp/jwt.h>
#include <jwt-cpp/traits/nlohmann-json/traits.h>

#include "../../http_client/tests/mocks/mock_http_client.hpp"

#include <boost/asio.hpp>

#include <chrono>
#include <functional>
#include <memory>
#include <string>

// NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)

using namespace testing;
using GetMessagesFuncType = std::function<boost::asio::awaitable<intStringTuple>(const size_t)>;

// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
MATCHER_P3(HttpRequestParamsCheck, expected, token, body, "Check http request params")
{
    auto test = expected;

    test.Token = token;
    test.Body = body;

    return arg == test;
}

namespace
{
    std::string CreateToken()
    {
        const auto now = std::chrono::system_clock::now();
        const auto exp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() + 10;

        return jwt::create<jwt::traits::nlohmann_json>()
            .set_issuer("auth0")
            .set_type("JWS")
            .set_payload_claim("exp", jwt::basic_claim<jwt::traits::nlohmann_json>(exp))
            .sign(jwt::algorithm::hs256 {"secret"});
    }

    const auto MOCK_CONFIG_PARSER = std::make_shared<configuration::ConfigurationParser>(std::string(R"(
        agent:
          retry_interval: 1s
    )"));

    const auto MOCK_CONFIG_PARSER_LOOP = std::make_shared<configuration::ConfigurationParser>(std::string(R"(
        agent:
          retry_interval: 5
          verification_mode: none
        events:
          batch_size: 1
    )"));
} // namespace

TEST(CommunicatorTest, CommunicatorConstructor)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();

    auto configurationParser = std::make_shared<configuration::ConfigurationParser>();

    EXPECT_NO_THROW(communicator::Communicator communicator(
        std::move(mockHttpClient), configurationParser, "uuid", "key", nullptr));
}

TEST(CommunicatorTest, CommunicatorConstructorNoHttpClient)
{
    auto configurationParser = std::make_shared<configuration::ConfigurationParser>();

    EXPECT_THROW(communicator::Communicator communicator(nullptr, configurationParser, "uuid", "key", nullptr),
                 std::runtime_error);
}

TEST(CommunicatorTest, CommunicatorConstructorNoConfigParser)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();

    EXPECT_THROW(communicator::Communicator communicator(std::move(mockHttpClient), nullptr, "uuid", "key", nullptr),
                 std::runtime_error);
}

TEST(CommunicatorTest, StatelessMessageProcessingTask_FailedAuthenticationLeavesEmptyToken)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    // not really a leak, as its lifetime is managed by the Communicator
    testing::Mock::AllowLeak(mockHttpClientPtr);

    auto communicatorPtr = std::make_shared<communicator::Communicator>(
        std::move(mockHttpClient), MOCK_CONFIG_PARSER_LOOP, "uuid", "key", nullptr);

    // A failed authentication won't return a token
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    intStringTuple expectedResponse {401, R"({"message":"Try again"})"};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_))
        .WillOnce(Invoke(
            [communicatorPtr, &expectedResponse]() -> intStringTuple
            {
                communicatorPtr->Stop();
                return expectedResponse;
            }));

    auto getMessagesCalled = false;
    auto onSuccessCalled = false;

    boost::asio::io_context ioContext;

    boost::asio::co_spawn(
        ioContext,
        [communicatorPtr, &getMessagesCalled, &onSuccessCalled]() mutable -> boost::asio::awaitable<void>
        {
            communicatorPtr->SendAuthenticationRequest();
            co_await communicatorPtr->StatelessMessageProcessingTask(
                [&getMessagesCalled](const size_t) -> boost::asio::awaitable<intStringTuple>
                {
                    getMessagesCalled = true;
                    co_return intStringTuple {1, std::string {"message"}};
                },
                [&onSuccessCalled](const int, const std::string&) { onSuccessCalled = true; });
        }(),
        boost::asio::detached);

    ioContext.run();

    EXPECT_FALSE(getMessagesCalled);
    EXPECT_FALSE(onSuccessCalled);
}

TEST(CommunicatorTest, StatelessMessageProcessingTask_CallsWithValidToken)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    // not really a leak, as its lifetime is managed by the Communicator
    testing::Mock::AllowLeak(mockHttpClientPtr);

    auto communicatorPtr = std::make_shared<communicator::Communicator>(
        std::move(mockHttpClient), MOCK_CONFIG_PARSER_LOOP, "uuid", "key", nullptr);

    const auto mockedToken = CreateToken();

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    intStringTuple expectedResponse1 {200, R"({"token":")" + mockedToken + R"("})"};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_))
        .WillOnce(Invoke([communicatorPtr, &expectedResponse1]() -> intStringTuple { return expectedResponse1; }));

    const auto reqParams = http_client::HttpRequestParams(
        http_client::MethodType::POST, "https://localhost:27000", "/api/v1/events/stateless", "", "none");

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    intStringTuple expectedResponse2 {200, "Dummy response"};

    EXPECT_CALL(*mockHttpClientPtr, Co_PerformHttpRequest(HttpRequestParamsCheck(reqParams, mockedToken, "message")))
        .WillOnce(Invoke(
            [communicatorPtr, &expectedResponse2]() -> boost::asio::awaitable<intStringTuple>
            {
                communicatorPtr->Stop();
                co_return expectedResponse2;
            }));

    auto getMessagesCalled = false;
    auto onSuccessCalled = false;

    boost::asio::io_context ioContext;

    boost::asio::co_spawn(
        ioContext,
        [communicatorPtr, &getMessagesCalled, &onSuccessCalled]() mutable -> boost::asio::awaitable<void>
        {
            communicatorPtr->SendAuthenticationRequest();
            co_await communicatorPtr->StatelessMessageProcessingTask(
                [&getMessagesCalled](const size_t) -> boost::asio::awaitable<intStringTuple>
                {
                    getMessagesCalled = true;
                    co_return intStringTuple {1, std::string {"message"}};
                },
                [&onSuccessCalled](const int, const std::string&) { onSuccessCalled = true; });
        }(),
        boost::asio::detached);

    ioContext.run();

    EXPECT_TRUE(getMessagesCalled);
    EXPECT_TRUE(onSuccessCalled);
}

TEST(CommunicatorTest, GetCommandsFromManager_CallsWithValidToken)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    // not really a leak, as its lifetime is managed by the Communicator
    testing::Mock::AllowLeak(mockHttpClientPtr);

    auto communicatorPtr = std::make_shared<communicator::Communicator>(
        std::move(mockHttpClient), MOCK_CONFIG_PARSER_LOOP, "uuid", "key", nullptr);

    const auto mockedToken = CreateToken();

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    intStringTuple expectedResponse1 {200, R"({"token":")" + mockedToken + R"("})"};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_))
        .WillOnce(Invoke([communicatorPtr, &expectedResponse1]() -> intStringTuple { return expectedResponse1; }));

    const auto reqParams = http_client::HttpRequestParams(
        http_client::MethodType::GET, "https://localhost:27000", "/api/v1/commands", "", "none", "", "", "", 60000);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    intStringTuple expectedResponse2 {200, "Dummy response"};

    EXPECT_CALL(*mockHttpClientPtr, Co_PerformHttpRequest(HttpRequestParamsCheck(reqParams, mockedToken, "")))
        .WillOnce(Invoke(
            [communicatorPtr, &expectedResponse2]() -> boost::asio::awaitable<intStringTuple>
            {
                communicatorPtr->Stop();
                co_return expectedResponse2;
            }));

    auto onSuccessCalled = false;

    boost::asio::io_context ioContext;

    boost::asio::co_spawn(
        ioContext,
        [communicatorPtr, &onSuccessCalled]() mutable -> boost::asio::awaitable<void>
        {
            communicatorPtr->SendAuthenticationRequest();
            co_await communicatorPtr->GetCommandsFromManager([&onSuccessCalled](const int, const std::string&)
                                                             { onSuccessCalled = true; });
        }(),
        boost::asio::detached);

    ioContext.run();

    EXPECT_TRUE(onSuccessCalled);
}

TEST(CommunicatorTest, GetCommandsFromManager_Failure)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    // not really a leak, as its lifetime is managed by the Communicator
    testing::Mock::AllowLeak(mockHttpClientPtr);

    auto communicatorPtr = std::make_shared<communicator::Communicator>(
        std::move(mockHttpClient), MOCK_CONFIG_PARSER_LOOP, "uuid", "key", nullptr);

    const auto mockedToken = CreateToken();

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    intStringTuple expectedResponse1 {200, R"({"token":")" + mockedToken + R"("})"};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_))
        .WillOnce(Invoke([communicatorPtr, &expectedResponse1]() -> intStringTuple { return expectedResponse1; }));

    const auto reqParams = http_client::HttpRequestParams(
        http_client::MethodType::GET, "https://localhost:27000", "/api/v1/commands", "", "none", "", "", "", 60000);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    intStringTuple expectedResponse2 {401, "Dummy response"};

    EXPECT_CALL(*mockHttpClientPtr, Co_PerformHttpRequest(HttpRequestParamsCheck(reqParams, mockedToken, "")))
        .WillOnce(Invoke(
            [communicatorPtr, &expectedResponse2]() -> boost::asio::awaitable<intStringTuple>
            {
                communicatorPtr->Stop();
                co_return expectedResponse2;
            }));

    auto onSuccessCalled = false;

    boost::asio::io_context ioContext;

    boost::asio::co_spawn(
        ioContext,
        [communicatorPtr, &onSuccessCalled]() mutable -> boost::asio::awaitable<void>
        {
            communicatorPtr->SendAuthenticationRequest();
            co_await communicatorPtr->GetCommandsFromManager([&onSuccessCalled](const int, const std::string&)
                                                             { onSuccessCalled = true; });
        }(),
        boost::asio::detached);

    ioContext.run();

    EXPECT_FALSE(onSuccessCalled);
}

TEST(CommunicatorTest, GetGroupConfigurationFromManager_Success)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    // not really a leak, as its lifetime is managed by the Communicator
    testing::Mock::AllowLeak(mockHttpClientPtr);

    std::string groupName = "group1";
    std::string dstFilePath = "./test-output";

    auto communicatorPtr = std::make_shared<communicator::Communicator>(
        std::move(mockHttpClient), MOCK_CONFIG_PARSER_LOOP, "uuid", "key", nullptr);

    const auto mockedToken = CreateToken();

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    intStringTuple expectedResponse1 {200, R"({"token":")" + mockedToken + R"("})"};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_))
        .WillOnce(Invoke([communicatorPtr, &expectedResponse1]() -> intStringTuple { return expectedResponse1; }));

    const auto reqParams = http_client::HttpRequestParams(
        http_client::MethodType::GET, "https://localhost:27000", "/api/v1/files?file_name=group1.yml", "", "none");

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    intStringTuple expectedResponse2 {200, "Dummy response"};

    EXPECT_CALL(*mockHttpClientPtr, Co_PerformHttpRequest(HttpRequestParamsCheck(reqParams, mockedToken, "")))
        .WillOnce(Invoke([communicatorPtr, &expectedResponse2]() -> boost::asio::awaitable<intStringTuple>
                         { co_return expectedResponse2; }));

    std::future<bool> result;

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        [&]() -> boost::asio::awaitable<void>
        {
            communicatorPtr->SendAuthenticationRequest();
            bool value = co_await communicatorPtr->GetGroupConfigurationFromManager(groupName, dstFilePath);
            std::promise<bool> promise;
            promise.set_value(value);
            result = promise.get_future();
        },
        boost::asio::detached);

    ioContext.run();
    EXPECT_TRUE(result.get());
}

TEST(CommunicatorTest, GetGroupConfigurationFromManager_Error)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    // not really a leak, as its lifetime is managed by the Communicator
    testing::Mock::AllowLeak(mockHttpClientPtr);

    std::string groupName = "group1";
    std::string dstFilePath = "dummy/non/existing/path";

    auto communicatorPtr = std::make_shared<communicator::Communicator>(
        std::move(mockHttpClient), MOCK_CONFIG_PARSER_LOOP, "uuid", "key", nullptr);

    const auto mockedToken = CreateToken();

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    intStringTuple expectedResponse1 {200, R"({"token":")" + mockedToken + R"("})"};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_))
        .WillOnce(Invoke([communicatorPtr, &expectedResponse1]() -> intStringTuple { return expectedResponse1; }));

    const auto reqParams = http_client::HttpRequestParams(
        http_client::MethodType::GET, "https://localhost:27000", "/api/v1/files?file_name=group1.yml", "", "none");

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    intStringTuple expectedResponse2 {200, "Dummy response"};

    EXPECT_CALL(*mockHttpClientPtr, Co_PerformHttpRequest(HttpRequestParamsCheck(reqParams, mockedToken, "")))
        .WillOnce(Invoke([communicatorPtr, &expectedResponse2]() -> boost::asio::awaitable<intStringTuple>
                         { co_return expectedResponse2; }));

    std::future<bool> result;

    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        [&]() -> boost::asio::awaitable<void>
        {
            communicatorPtr->SendAuthenticationRequest();
            bool value = co_await communicatorPtr->GetGroupConfigurationFromManager(groupName, dstFilePath);
            std::promise<bool> promise;
            promise.set_value(value);
            result = promise.get_future();
        },
        boost::asio::detached);

    ioContext.run();
    EXPECT_FALSE(result.get());
}

TEST(CommunicatorTest, AuthenticateWithUuidAndKey_Success)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    auto communicatorPtr = std::make_shared<communicator::Communicator>(
        std::move(mockHttpClient), MOCK_CONFIG_PARSER, "uuid", "key", nullptr);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    intStringTuple expectedResponse {200, R"({"token":"valid_token"})"};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    const auto token = communicatorPtr->AuthenticateWithUuidAndKey();

    ASSERT_TRUE(token.has_value());

    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(token.value(), "valid_token");
}

TEST(CommunicatorTest, AuthenticateWithUuidAndKey_Failure)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    auto communicatorPtr = std::make_shared<communicator::Communicator>(
        std::move(mockHttpClient), MOCK_CONFIG_PARSER, "uuid", "key", nullptr);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    intStringTuple expectedResponse {401, R"({"message":"Try again"})"};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    const auto token = communicatorPtr->AuthenticateWithUuidAndKey();

    EXPECT_FALSE(token.has_value());
}

TEST(CommunicatorTest, AuthenticateWithUuidAndKey_FailureThrowsException)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    auto communicatorPtr = std::make_shared<communicator::Communicator>(
        std::move(mockHttpClient), MOCK_CONFIG_PARSER, "uuid", "key", nullptr);

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    intStringTuple expectedResponse {401, R"({"message":"Invalid key"})"};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    EXPECT_THROW(communicatorPtr->AuthenticateWithUuidAndKey(), std::runtime_error);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)
