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

TEST(CommunicatorTest, StatefulMessageProcessingTask_Success)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();

    auto getMessages = [](const size_t) -> boost::asio::awaitable<intStringTuple>
    {
        co_return intStringTuple {1, std::string("message-content")};
    };

    std::function<void(const int, const std::string&)> onSuccess = [](const int, const std::string& message)
    {
        EXPECT_EQ(message, "message-content");
    };

    auto MockCo_PerformHttpRequest =
        [](std::shared_ptr<std::string>,
           http_client::HttpRequestParams,
           GetMessagesFuncType pGetMessages,
           std::function<void()>,
           [[maybe_unused]] std::time_t connectionRetry,
           [[maybe_unused]] size_t batchSize,
           std::function<void(const int, const std::string&)> pOnSuccess,
           [[maybe_unused]] std::function<bool()> loopRequestCondition) -> boost::asio::awaitable<void>
    {
        const auto message = co_await pGetMessages(1);
        pOnSuccess(std::get<0>(message), std::get<1>(message));
        co_return;
    };

    EXPECT_CALL(*mockHttpClient, Co_PerformHttpRequest(_, _, _, _, _, _, _, _))
        .WillOnce(Invoke(MockCo_PerformHttpRequest));

    communicator::Communicator communicator(std::move(mockHttpClient), MOCK_CONFIG_PARSER, "uuid", "key", nullptr);

    auto task = communicator.StatefulMessageProcessingTask(getMessages, onSuccess);
    boost::asio::io_context ioContext;
    boost::asio::co_spawn(ioContext, std::move(task), boost::asio::detached);
    ioContext.run();
}

TEST(CommunicatorTest, WaitForTokenExpirationAndAuthenticate_FailedAuthenticationLeavesEmptyToken)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    // not really a leak, as its lifetime is managed by the Communicator
    testing::Mock::AllowLeak(mockHttpClientPtr);

    auto communicatorPtr = std::make_shared<communicator::Communicator>(
        std::move(mockHttpClient), MOCK_CONFIG_PARSER, "uuid", "key", nullptr);

    // A failed authentication won't return a token
    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::tuple<int, std::string> expectedResponse {401, R"({"message":"Try again"})"};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_))
        .WillOnce(Invoke(
            [communicatorPtr, &expectedResponse]() -> std::tuple<int, std::string>
            {
                communicatorPtr->Stop();
                return expectedResponse;
            }));

    auto MockCo_PerformHttpRequest =
        [](std::shared_ptr<std::string> token,
           http_client::HttpRequestParams,
           [[maybe_unused]] GetMessagesFuncType pGetMessages,
           [[maybe_unused]] std::function<void()> onUnauthorized,
           [[maybe_unused]] std::time_t connectionRetry,
           [[maybe_unused]] size_t batchSize,
           [[maybe_unused]] std::function<void(const int, const std::string&)> onSuccess,
           [[maybe_unused]] std::function<bool()> loopCondition) -> boost::asio::awaitable<void>
    {
        EXPECT_TRUE(token->empty());
        co_return;
    };

    // A following call to Co_PerformHttpRequest should not have a token
    EXPECT_CALL(*mockHttpClientPtr, Co_PerformHttpRequest(_, _, _, _, _, _, _, _))
        .WillOnce(Invoke(MockCo_PerformHttpRequest));

    boost::asio::io_context ioContext;

    boost::asio::co_spawn(
        ioContext,
        [communicatorPtr]() mutable -> boost::asio::awaitable<void>
        {
            co_await communicatorPtr->WaitForTokenExpirationAndAuthenticate();
            co_await communicatorPtr->StatelessMessageProcessingTask(
                [](const size_t) -> boost::asio::awaitable<intStringTuple>
                { co_return intStringTuple(1, std::string {"message"}); },
                []([[maybe_unused]] const int, const std::string&) {});
        }(),
        boost::asio::detached);

    ioContext.run();
}

TEST(CommunicatorTest, StatelessMessageProcessingTask_CallsWithValidToken)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    // not really a leak, as its lifetime is managed by the Communicator
    testing::Mock::AllowLeak(mockHttpClientPtr);

    auto communicatorPtr = std::make_shared<communicator::Communicator>(
        std::move(mockHttpClient), MOCK_CONFIG_PARSER, "uuid", "key", nullptr);

    const auto mockedToken = CreateToken();

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::tuple<int, std::string> expectedResponse {200, R"({"token":")" + mockedToken + R"("})"};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_))
        .WillOnce(Invoke(
            [communicatorPtr, &expectedResponse]() -> std::tuple<int, std::string>
            {
                communicatorPtr->Stop();
                return expectedResponse;
            }));

    std::string capturedToken;

    auto MockCo_PerformHttpRequest =
        [&capturedToken](std::shared_ptr<std::string> token,
                         http_client::HttpRequestParams,
                         [[maybe_unused]] GetMessagesFuncType pGetMessages,
                         [[maybe_unused]] std::function<void()> onUnauthorized,
                         [[maybe_unused]] std::time_t connectionRetry,
                         [[maybe_unused]] size_t batchSize,
                         [[maybe_unused]] std::function<void(const int, const std::string&)> onSuccess,
                         [[maybe_unused]] std::function<bool()> loopCondition) -> boost::asio::awaitable<void>
    {
        capturedToken = *token;
        co_return;
    };

    EXPECT_CALL(*mockHttpClientPtr, Co_PerformHttpRequest(_, _, _, _, _, _, _, _))
        .WillOnce(Invoke(MockCo_PerformHttpRequest));

    boost::asio::io_context ioContext;

    boost::asio::co_spawn(
        ioContext,
        [communicatorPtr]() mutable -> boost::asio::awaitable<void>
        {
            co_await communicatorPtr->WaitForTokenExpirationAndAuthenticate();
            co_await communicatorPtr->StatelessMessageProcessingTask(
                [](const size_t) -> boost::asio::awaitable<intStringTuple>
                { co_return intStringTuple(1, std::string {"message"}); },
                []([[maybe_unused]] const int, const std::string&) {});
        }(),
        boost::asio::detached);

    ioContext.run();

    EXPECT_FALSE(capturedToken.empty());
    EXPECT_EQ(capturedToken, mockedToken);
}

TEST(CommunicatorTest, GetGroupConfigurationFromManager_Success)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    // not really a leak, as its lifetime is managed by the Communicator
    testing::Mock::AllowLeak(mockHttpClientPtr);

    std::string groupName = "group1";
    std::string dstFilePath = "./test-output";

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::tuple<int, std::string> expectedResponse {200, ""};

    // NOLINTBEGIN(cppcoreguidelines-avoid-reference-coroutine-parameters)
    auto MockCo_PerformHttpRequest =
        [](std::shared_ptr<std::string>,
           const http_client::HttpRequestParams&,
           const GetMessagesFuncType&,
           const std::function<void()>&,
           [[maybe_unused]] std::time_t connectionRetry,
           [[maybe_unused]] size_t batchSize,
           [[maybe_unused]] std::function<void(const int, const std::string&)> pOnSuccess,
           [[maybe_unused]] std::function<bool()> loopRequestCondition) -> boost::asio::awaitable<void>
    {
        pOnSuccess(200, "Dummy response"); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        co_return;
    };
    // NOLINTEND(cppcoreguidelines-avoid-reference-coroutine-parameters)

    EXPECT_CALL(*mockHttpClient, Co_PerformHttpRequest(_, _, _, _, _, _, _, _))
        .WillOnce(Invoke(MockCo_PerformHttpRequest));

    communicator::Communicator communicator(std::move(mockHttpClient), MOCK_CONFIG_PARSER, "uuid", "key", nullptr);

    std::future<bool> result;

    auto task = communicator.GetGroupConfigurationFromManager(groupName, dstFilePath);
    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        [&]() -> boost::asio::awaitable<void>
        {
            bool value = co_await communicator.GetGroupConfigurationFromManager(groupName, dstFilePath);
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

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::tuple<int, std::string> expectedResponse {200, ""};

    // NOLINTBEGIN(cppcoreguidelines-avoid-reference-coroutine-parameters)
    auto MockCo_PerformHttpRequest =
        [](std::shared_ptr<std::string>,
           const http_client::HttpRequestParams&,
           const GetMessagesFuncType&,
           const std::function<void()>&,
           [[maybe_unused]] std::time_t connectionRetry,
           [[maybe_unused]] size_t batchSize,
           [[maybe_unused]] std::function<void(const int, const std::string&)> pOnSuccess,
           [[maybe_unused]] std::function<bool()> loopRequestCondition) -> boost::asio::awaitable<void>
    {
        pOnSuccess(200, "Dummy response"); // NOLINT(cppcoreguidelines-avoid-magic-numbers)
        co_return;
    };
    // NOLINTEND(cppcoreguidelines-avoid-reference-coroutine-parameters)

    EXPECT_CALL(*mockHttpClient, Co_PerformHttpRequest(_, _, _, _, _, _, _, _))
        .WillOnce(Invoke(MockCo_PerformHttpRequest));

    communicator::Communicator communicator(std::move(mockHttpClient), MOCK_CONFIG_PARSER, "uuid", "key", nullptr);

    std::future<bool> result;

    auto task = communicator.GetGroupConfigurationFromManager(groupName, dstFilePath);
    boost::asio::io_context ioContext;
    boost::asio::co_spawn(
        ioContext,
        [&]() -> boost::asio::awaitable<void>
        {
            bool value = co_await communicator.GetGroupConfigurationFromManager(groupName, dstFilePath);
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
    std::tuple<int, std::string> expectedResponse {200, R"({"token":"valid_token"})"};

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
    std::tuple<int, std::string> expectedResponse {401, R"({"message":"Try again"})"};

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
    std::tuple<int, std::string> expectedResponse {401, R"({"message":"Invalid key"})"};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    EXPECT_THROW(communicatorPtr->AuthenticateWithUuidAndKey(), std::runtime_error);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)
