#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <communicator.hpp>
#include <ihttp_client.hpp>

#include "mocks/mock_http_client.hpp"

using namespace testing;

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
            [](const std::string&,
               http_client::HttpRequestParams,
               std::function<boost::asio::awaitable<std::string>()> pGetMessages,
               std::function<void()>,
               std::function<void(const std::string&)> pOnSuccess,
               std::function<bool()> loopRequestCondition) -> boost::asio::awaitable<void>
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
            [communicatorPtr](const std::string& host,
                              const std::string& port,
                              const std::string& uuid,
                              const std::string& key) -> std::optional<std::string>
            {
                communicatorPtr->Stop();
                return std::nullopt;
            }));

    // A following call to Co_PerformHttpRequest should not have a token
    EXPECT_CALL(*mockHttpClientPtr, Co_PerformHttpRequest(_, _, _, _, _, _))
        .WillOnce(Invoke(
            [](const std::string& token,
               http_client::HttpRequestParams,
               std::function<boost::asio::awaitable<std::string>()> getMessages,
               std::function<void()> onUnauthorized,
               std::function<void(const std::string&)> onSuccess,
               std::function<bool()> loopCondition) -> boost::asio::awaitable<void>
            {
                EXPECT_TRUE(token.empty());
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
            co_await communicatorPtr->StatelessMessageProcessingTask(
                []() -> boost::asio::awaitable<std::string> { co_return "message"; }, [](const std::string& msg) {});
        },
        boost::asio::detached);

    ioContext.run();
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
