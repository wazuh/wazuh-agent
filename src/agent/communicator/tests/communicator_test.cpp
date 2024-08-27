#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <communicator.hpp>

#include <ihttp_client.hpp>

using namespace testing;

class MockHttpClient : public http_client::IHttpClient
{
public:
    MOCK_METHOD(boost::beast::http::request<boost::beast::http::string_body>,
                CreateHttpRequest,
                (const http_client::HttpRequestParams& params),
                (override));

    MOCK_METHOD(boost::asio::awaitable<void>,
                Co_PerformHttpRequest,
                (const std::string& token,
                 http_client::HttpRequestParams params,
                 std::function<boost::asio::awaitable<std::string>()> messageGetter,
                 std::function<void()> onUnauthorized,
                 std::function<void(const std::string&)> onSuccess,
                 std::function<bool()> loopRequestCondition),
                (override));

    MOCK_METHOD(boost::beast::http::response<boost::beast::http::dynamic_body>,
                PerformHttpRequest,
                (const http_client::HttpRequestParams& params),
                (override));

    MOCK_METHOD(std::optional<std::string>,
                AuthenticateWithUuidAndKey,
                (const std::string& host, const std::string& port, const std::string& uuid, const std::string& key),
                (override));

    MOCK_METHOD(
        std::optional<std::string>,
        AuthenticateWithUserPassword,
        (const std::string& host, const std::string& port, const std::string& user, const std::string& password),
        (override));
};

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
