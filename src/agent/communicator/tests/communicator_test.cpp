#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <communicator.hpp>
#include <ihttp_client.hpp>

#include <jwt-cpp/jwt.h>

#include "mocks/mock_http_client.hpp"

// NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)

using namespace testing;

namespace
{
    std::string CreateToken()
    {
        const auto now = std::chrono::system_clock::now();
        const auto exp = std::chrono::duration_cast<std::chrono::seconds>(now.time_since_epoch()).count() + 3600;

        return jwt::create()
            .set_issuer("auth0")
            .set_type("JWS")
            .set_payload_claim("exp", jwt::claim(std::to_string(exp)))
            .sign(jwt::algorithm::hs256 {"secret"});
    }
} // namespace

TEST(CommunicatorTest, CommunicatorConstructor)
{
    EXPECT_NO_THROW(communicator::Communicator communicator(nullptr, "uuid", "key", nullptr));
}

TEST(CommunicatorTest, StatefulMessageProcessingTask_Success)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();

    auto getMessages = []() -> boost::asio::awaitable<std::string>
    {
        co_return std::string("message-content");
    };

    std::function<void(const std::string&)> onSuccess = [](const std::string& message)
    {
        EXPECT_EQ(message, "message-content");
    };

    EXPECT_CALL(*mockHttpClient, Co_PerformHttpRequest(_, _, _, _, _, _))
        .WillOnce(Invoke(
            [](std::shared_ptr<std::string>,
               http_client::HttpRequestParams,
               std::function<boost::asio::awaitable<std::string>()> pGetMessages,
               std::function<void()>,
               std::function<void(const std::string&)> pOnSuccess,
               [[maybe_unused]] std::function<bool()> loopRequestCondition) -> boost::asio::awaitable<void>
            {
                const auto message = co_await pGetMessages();
                pOnSuccess(message);
                co_return;
            }));

    communicator::Communicator communicator(std::move(mockHttpClient), "uuid", "key", nullptr);

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

    auto communicatorPtr =
        std::make_shared<communicator::Communicator>(std::move(mockHttpClient), "uuid", "key", nullptr);

    // A failed authentication won't return a token
    EXPECT_CALL(*mockHttpClientPtr, AuthenticateWithUuidAndKey(_, _, _, _))
        .WillOnce(Invoke(
            [communicatorPtr]([[maybe_unused]] const std::string& host,
                              [[maybe_unused]] const std::string& port,
                              [[maybe_unused]] const std::string& uuid,
                              [[maybe_unused]] const std::string& key) -> std::optional<std::string>
            {
                communicatorPtr->Stop();
                return std::nullopt;
            }));

    // A following call to Co_PerformHttpRequest should not have a token
    EXPECT_CALL(*mockHttpClientPtr, Co_PerformHttpRequest(_, _, _, _, _, _))
        .WillOnce(Invoke(
            [](std::shared_ptr<std::string> token,
               http_client::HttpRequestParams,
               [[maybe_unused]] std::function<boost::asio::awaitable<std::string>()> getMessages,
               [[maybe_unused]] std::function<void()> onUnauthorized,
               [[maybe_unused]] std::function<void(const std::string&)> onSuccess,
               [[maybe_unused]] std::function<bool()> loopCondition) -> boost::asio::awaitable<void>
            {
                EXPECT_TRUE(token->empty());
                co_return;
            }));

    boost::asio::io_context ioContext;

    boost::asio::co_spawn(
        ioContext,
        [communicatorPtr]() mutable -> boost::asio::awaitable<void>
        { co_await communicatorPtr->WaitForTokenExpirationAndAuthenticate(); },
        boost::asio::detached);

    boost::asio::co_spawn(
        ioContext,
        [communicatorPtr]() mutable -> boost::asio::awaitable<void>
        {
            co_await communicatorPtr->StatelessMessageProcessingTask([]() -> boost::asio::awaitable<std::string>
                                                                     { co_return "message"; },
                                                                     []([[maybe_unused]] const std::string& msg) {});
        },
        boost::asio::detached);

    ioContext.run();
}

TEST(CommunicatorTest, StatelessMessageProcessingTask_CallsWithValidToken)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    // not really a leak, as its lifetime is managed by the Communicator
    testing::Mock::AllowLeak(mockHttpClientPtr);

    auto communicatorPtr =
        std::make_shared<communicator::Communicator>(std::move(mockHttpClient), "uuid", "key", nullptr);

    const auto mockedToken = CreateToken();
    EXPECT_CALL(*mockHttpClientPtr, AuthenticateWithUuidAndKey(_, _, _, _)).WillOnce(Return(mockedToken));

    std::string capturedToken;
    EXPECT_CALL(*mockHttpClientPtr, Co_PerformHttpRequest(_, _, _, _, _, _))
        .WillOnce(Invoke(
            [&capturedToken](std::shared_ptr<std::string> token,
                             http_client::HttpRequestParams,
                             [[maybe_unused]] std::function<boost::asio::awaitable<std::string>()> getMessages,
                             [[maybe_unused]] std::function<void()> onUnauthorized,
                             [[maybe_unused]] std::function<void(const std::string&)> onSuccess,
                             [[maybe_unused]] std::function<bool()> loopCondition) -> boost::asio::awaitable<void>
            {
                capturedToken = *token;
                co_return;
            }));

    boost::asio::io_context ioContext;

    boost::asio::co_spawn(
        ioContext,
        [communicatorPtr]() mutable -> boost::asio::awaitable<void>
        { co_await communicatorPtr->WaitForTokenExpirationAndAuthenticate(); },
        boost::asio::detached);

    boost::asio::co_spawn(
        ioContext,
        [communicatorPtr]() mutable -> boost::asio::awaitable<void>
        {
            co_await communicatorPtr->StatelessMessageProcessingTask([]() -> boost::asio::awaitable<std::string>
                                                                     { co_return "message"; },
                                                                     []([[maybe_unused]] const std::string& msg) {});
        },
        boost::asio::detached);

    ioContext.run();

    EXPECT_FALSE(capturedToken.empty());
    EXPECT_EQ(capturedToken, mockedToken);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)
