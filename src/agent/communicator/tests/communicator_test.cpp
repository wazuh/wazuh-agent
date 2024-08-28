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

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
