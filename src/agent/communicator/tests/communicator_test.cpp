#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <communicator.hpp>
#include <http_request_params.hpp>
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

    const auto MOCK_CONFIG_PARSER_LOOP = std::make_shared<configuration::ConfigurationParser>(std::string(R"(
        agent:
          retry_interval: 5
          verification_mode: none
        events:
          batch_size: 1
    )"));

    void SpawnCoroutine(std::function<boost::asio::awaitable<void>()> func)
    {
        boost::asio::io_context ioContext;
        boost::asio::co_spawn(ioContext, std::move(func), boost::asio::detached);
        ioContext.run();
    }
} // namespace

class CommunicatorTest : public ::testing::Test
{
protected:
    void SetUp() override
    {

        auto mockHttpClient = std::make_unique<MockHttpClient>();
        m_mockHttpClientPtr = mockHttpClient.get();
        testing::Mock::AllowLeak(m_mockHttpClientPtr);
        m_mockedToken = CreateToken();

        m_communicator = std::make_shared<communicator::Communicator>(
            std::move(mockHttpClient), MOCK_CONFIG_PARSER_LOOP, "uuid", "key", nullptr);

        EXPECT_CALL(*m_mockHttpClientPtr, PerformHttpRequest(testing::_))
            .WillRepeatedly(Invoke([token = m_mockedToken]() -> intStringTuple
                                   { return {http_client::HTTP_CODE_OK, R"({"token":")" + token + R"("})"}; }));
    }

    void TearDown() override
    {
        m_mockedToken.clear();
        m_mockHttpClientPtr = nullptr;
        m_communicator = nullptr;
    }

    std::shared_ptr<communicator::Communicator> m_communicator = nullptr;
    MockHttpClient* m_mockHttpClientPtr = nullptr;
    std::string m_mockedToken;
};

TEST_F(CommunicatorTest, CommunicatorConstructor)
{
    EXPECT_NO_THROW(communicator::Communicator communicator(std::make_unique<MockHttpClient>(),
                                                            std::make_shared<configuration::ConfigurationParser>(),
                                                            "uuid",
                                                            "key",
                                                            nullptr));
}

TEST_F(CommunicatorTest, CommunicatorConstructorNoHttpClient)
{
    EXPECT_THROW(communicator::Communicator communicator(
                     nullptr, std::make_shared<configuration::ConfigurationParser>(), "uuid", "key", nullptr),
                 std::runtime_error);
}

TEST_F(CommunicatorTest, CommunicatorConstructorNoConfigParser)
{
    EXPECT_THROW(
        communicator::Communicator communicator(std::make_unique<MockHttpClient>(), nullptr, "uuid", "key", nullptr),
        std::runtime_error);
}

TEST_F(CommunicatorTest, StatelessMessageProcessingTask_FailedAuthenticationLeavesEmptyToken)
{
    EXPECT_CALL(*m_mockHttpClientPtr, PerformHttpRequest(testing::_))
        .WillOnce(Invoke(
            [this]() -> intStringTuple
            {
                m_communicator->Stop();

                // A failed authentication won't return a token
                return {http_client::HTTP_CODE_UNAUTHORIZED, R"({"message":"Try again"})"};
            }));

    auto getMessagesCalled = false;
    auto onSuccessCalled = false;

    SpawnCoroutine(
        [this, &getMessagesCalled, &onSuccessCalled]() mutable -> boost::asio::awaitable<void>
        {
            m_communicator->SendAuthenticationRequest();
            co_await m_communicator->StatelessMessageProcessingTask(
                [&getMessagesCalled](const size_t) -> boost::asio::awaitable<intStringTuple>
                {
                    getMessagesCalled = true;
                    co_return intStringTuple {1, std::string {"message"}};
                },
                [&onSuccessCalled](const int, const std::string&) { onSuccessCalled = true; });
        });

    EXPECT_FALSE(getMessagesCalled);
    EXPECT_FALSE(onSuccessCalled);
}

TEST_F(CommunicatorTest, StatelessMessageProcessingTask_CallsWithValidToken)
{
    const auto reqParams = http_client::HttpRequestParams(
        http_client::MethodType::POST, "https://localhost:27000", "/api/v1/events/stateless", "", "none");

    EXPECT_CALL(*m_mockHttpClientPtr,
                Co_PerformHttpRequest(HttpRequestParamsCheck(reqParams, m_mockedToken, "message")))
        .WillOnce(Invoke(
            [this]() -> boost::asio::awaitable<intStringTuple>
            {
                m_communicator->Stop();
                co_return intStringTuple {http_client::HTTP_CODE_OK, "Dummy response"};
            }));

    auto getMessagesCalled = false;
    auto onSuccessCalled = false;

    SpawnCoroutine(
        [this, &getMessagesCalled, &onSuccessCalled]() mutable -> boost::asio::awaitable<void>
        {
            m_communicator->SendAuthenticationRequest();
            co_await m_communicator->StatelessMessageProcessingTask(
                [&getMessagesCalled](const size_t) -> boost::asio::awaitable<intStringTuple>
                {
                    getMessagesCalled = true;
                    co_return intStringTuple {1, std::string {"message"}};
                },
                [&onSuccessCalled](const int, const std::string&) { onSuccessCalled = true; });
        });

    EXPECT_TRUE(getMessagesCalled);
    EXPECT_TRUE(onSuccessCalled);
}

TEST_F(CommunicatorTest, GetCommandsFromManager_CallsWithValidToken)
{
    const auto timeout = static_cast<time_t>(11) * 60 * 1000;
    const auto reqParams = http_client::HttpRequestParams(
        http_client::MethodType::GET, "https://localhost:27000", "/api/v1/commands", "", "none", "", "", "", timeout);

    EXPECT_CALL(*m_mockHttpClientPtr, Co_PerformHttpRequest(HttpRequestParamsCheck(reqParams, m_mockedToken, "")))
        .WillOnce(Invoke(
            [this]() -> boost::asio::awaitable<intStringTuple>
            {
                m_communicator->Stop();
                co_return intStringTuple {http_client::HTTP_CODE_OK, "Dummy response"};
            }));

    auto onSuccessCalled = false;

    SpawnCoroutine(
        [this, &onSuccessCalled]() mutable -> boost::asio::awaitable<void>
        {
            m_communicator->SendAuthenticationRequest();
            co_await m_communicator->GetCommandsFromManager([&onSuccessCalled](const int, const std::string&)
                                                            { onSuccessCalled = true; });
        });

    EXPECT_TRUE(onSuccessCalled);
}

TEST_F(CommunicatorTest, GetCommandsFromManager_Failure)
{
    const auto timeout = static_cast<time_t>(11) * 60 * 1000;
    const auto reqParams = http_client::HttpRequestParams(
        http_client::MethodType::GET, "https://localhost:27000", "/api/v1/commands", "", "none", "", "", "", timeout);

    EXPECT_CALL(*m_mockHttpClientPtr, Co_PerformHttpRequest(HttpRequestParamsCheck(reqParams, m_mockedToken, "")))
        .WillOnce(Invoke(
            [this]() -> boost::asio::awaitable<intStringTuple>
            {
                m_communicator->Stop();
                co_return intStringTuple {http_client::HTTP_CODE_UNAUTHORIZED, "Dummy response"};
            }));

    auto onSuccessCalled = false;

    SpawnCoroutine(
        [this, &onSuccessCalled]() mutable -> boost::asio::awaitable<void>
        {
            m_communicator->SendAuthenticationRequest();
            co_await m_communicator->GetCommandsFromManager([&onSuccessCalled](const int, const std::string&)
                                                            { onSuccessCalled = true; });
        });

    EXPECT_FALSE(onSuccessCalled);
}

TEST_F(CommunicatorTest, GetGroupConfigurationFromManager_Success)
{
    const auto reqParams = http_client::HttpRequestParams(
        http_client::MethodType::GET, "https://localhost:27000", "/api/v1/files?file_name=group1.yml", "", "none");

    EXPECT_CALL(*m_mockHttpClientPtr, Co_PerformHttpRequest(HttpRequestParamsCheck(reqParams, m_mockedToken, "")))
        .WillOnce(Invoke([]() -> boost::asio::awaitable<intStringTuple>
                         { co_return intStringTuple {http_client::HTTP_CODE_OK, "Dummy response"}; }));

    auto result = false;

    SpawnCoroutine(
        [this, &result]() -> boost::asio::awaitable<void>
        {
            m_communicator->SendAuthenticationRequest();

            const std::string groupName = "group1";
            const std::string dstFilePath = "./test-output";
            result = co_await m_communicator->GetGroupConfigurationFromManager(groupName, dstFilePath);
        });

    EXPECT_TRUE(result);
}

TEST_F(CommunicatorTest, GetGroupConfigurationFromManager_Error)
{
    const auto reqParams = http_client::HttpRequestParams(
        http_client::MethodType::GET, "https://localhost:27000", "/api/v1/files?file_name=group1.yml", "", "none");

    EXPECT_CALL(*m_mockHttpClientPtr, Co_PerformHttpRequest(HttpRequestParamsCheck(reqParams, m_mockedToken, "")))
        .WillOnce(Invoke([]() -> boost::asio::awaitable<intStringTuple>
                         { co_return intStringTuple {http_client::HTTP_CODE_OK, "Dummy response"}; }));

    auto result = false;

    SpawnCoroutine(
        [&]() -> boost::asio::awaitable<void>
        {
            m_communicator->SendAuthenticationRequest();

            const std::string groupName = "group1";
            const std::string dstFilePath = "dummy/non/existing/path";
            result = co_await m_communicator->GetGroupConfigurationFromManager(groupName, dstFilePath);
        });

    EXPECT_FALSE(result);
}

TEST_F(CommunicatorTest, AuthenticateWithUuidAndKey_Success)
{
    const intStringTuple expectedResponse {http_client::HTTP_CODE_OK, R"({"token":"valid_token"})"};

    EXPECT_CALL(*m_mockHttpClientPtr, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    const auto token = m_communicator->AuthenticateWithUuidAndKey();

    ASSERT_TRUE(token.has_value());

    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(token.value(), "valid_token");
}

TEST_F(CommunicatorTest, AuthenticateWithUuidAndKey_Failure)
{
    const intStringTuple expectedResponse {http_client::HTTP_CODE_UNAUTHORIZED, R"({"message":"Try again"})"};

    EXPECT_CALL(*m_mockHttpClientPtr, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    const auto token = m_communicator->AuthenticateWithUuidAndKey();

    EXPECT_FALSE(token.has_value());
}

TEST_F(CommunicatorTest, AuthenticateWithUuidAndKey_FailureThrowsException)
{
    const intStringTuple expectedResponse {http_client::HTTP_CODE_UNAUTHORIZED, R"({"message":"Invalid key"})"};

    EXPECT_CALL(*m_mockHttpClientPtr, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    EXPECT_THROW(m_communicator->AuthenticateWithUuidAndKey(), std::runtime_error);
}

TEST_F(CommunicatorTest, SendAgentShutdownMessagesDoesNothingIfNotAuthenticated)
{
    EXPECT_CALL(*m_mockHttpClientPtr, PerformHttpRequest(testing::_)).Times(0);

    SpawnCoroutine(
        [this]() -> boost::asio::awaitable<void>
        {
            m_communicator->SendAgentShutdownMessage("");
            co_return;
        });
}

TEST_F(CommunicatorTest, SendAgentShutdownMessageSendsStatelessMessage)
{
    const intStringTuple expectedResponse {http_client::HTTP_CODE_OK, "dummy response"};

    EXPECT_CALL(*m_mockHttpClientPtr, PerformHttpRequest(testing::_))
        .Times(1)
        .WillOnce(Invoke([token = m_mockedToken]() -> intStringTuple
                         { return {http_client::HTTP_CODE_OK, R"({"token":")" + token + R"("})"}; }));

    EXPECT_CALL(*m_mockHttpClientPtr,
                PerformHttpRequest(testing::Truly(
                    [](const http_client::HttpRequestParams& params) {
                        return params.Endpoint == "/api/v1/events/stateless" &&
                               params.Body.find("shutdown") != std::string::npos;
                    })))
        .WillOnce(testing::Return(expectedResponse));

    EXPECT_CALL(*m_mockHttpClientPtr,
                PerformHttpRequest(testing::Truly(
                    [](const http_client::HttpRequestParams& params) {
                        return params.Endpoint == "/api/v1/events/stateful" &&
                               params.Body.find("shutdown") != std::string::npos;
                    })))
        .WillOnce(testing::Return(expectedResponse));

    SpawnCoroutine(
        [this]() -> boost::asio::awaitable<void>
        {
            m_communicator->SendAuthenticationRequest();
            m_communicator->SendAgentShutdownMessage("");
            co_return;
        });
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}

// NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)
