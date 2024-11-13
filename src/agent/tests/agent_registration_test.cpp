#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <agent_info.hpp>
#include <agent_info_persistance.hpp>
#include <agent_registration.hpp>
#include <ihttp_client.hpp>
#include <ihttp_socket.hpp>

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

#include <memory>
#include <optional>
#include <string>

class MockHttpClient : public http_client::IHttpClient
{
public:
    MOCK_METHOD(boost::beast::http::request<boost::beast::http::string_body>,
                CreateHttpRequest,
                (const http_client::HttpRequestParams& params),
                (override));

    MOCK_METHOD(boost::asio::awaitable<void>,
                Co_PerformHttpRequest,
                (std::shared_ptr<std::string> token,
                 http_client::HttpRequestParams params,
                 std::function<boost::asio::awaitable<std::string>()> messageGetter,
                 std::function<void()> onUnauthorized,
                 long connectionRetrySecs,
                 std::function<void(const std::string&)> onSuccess,
                 std::function<bool()> loopRequestCondition),
                (override));

    MOCK_METHOD(boost::beast::http::response<boost::beast::http::dynamic_body>,
                PerformHttpRequest,
                (const http_client::HttpRequestParams& params),
                (override));

    MOCK_METHOD(
        std::optional<std::string>,
        AuthenticateWithUuidAndKey,
        (const std::string& host, const std::string& userAgent, const std::string& uuid, const std::string& key),
        (override));

    MOCK_METHOD(
        std::optional<std::string>,
        AuthenticateWithUserPassword,
        (const std::string& host, const std::string& userAgent, const std::string& user, const std::string& password),
        (override));

    MOCK_METHOD(boost::beast::http::response<boost::beast::http::dynamic_body>,
                PerformHttpRequestDownload,
                (const http_client::HttpRequestParams& params, const std::string& dstFilePath),
                (override));
};

class RegisterTest : public ::testing::Test
{
protected:
    void SetUp() override {}

    std::unique_ptr<AgentInfo> agent;
    std::unique_ptr<agent_registration::AgentRegistration> registration;
};

TEST_F(RegisterTest, RegistrationTestSuccess)
{
    AgentInfoPersistance agentInfoPersistance;
    agentInfoPersistance.ResetToDefault();

    registration = std::make_unique<agent_registration::AgentRegistration>(
        "user", "password", "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj", "agent_name", std::nullopt);
    SysInfo sysInfo;
    agent = std::make_unique<AgentInfo>([&sysInfo]() mutable { return sysInfo.os(); },
                                        [&sysInfo]() mutable { return sysInfo.networks(); });

    MockHttpClient mockHttpClient;

    EXPECT_CALL(mockHttpClient, AuthenticateWithUserPassword(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return("token"));

    const auto bodyJson = agent->GetMetadataInfo(true);

    http_client::HttpRequestParams reqParams(boost::beast::http::verb::post,
                                             "https://localhost:55000",
                                             "/agents",
                                             agent->GetHeaderInfo(),
                                             "token",
                                             "",
                                             bodyJson);

    boost::beast::http::response<boost::beast::http::dynamic_body> expectedResponse;
    expectedResponse.result(boost::beast::http::status::ok);

    EXPECT_CALL(mockHttpClient, PerformHttpRequest(testing::Eq(reqParams))).WillOnce(testing::Return(expectedResponse));

    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    const bool res = registration->Register(mockHttpClient);
    ASSERT_TRUE(res);
}

TEST_F(RegisterTest, RegistrationFailsIfAuthenticationFails)
{
    AgentInfoPersistance agentInfoPersistance;
    agentInfoPersistance.ResetToDefault();

    registration = std::make_unique<agent_registration::AgentRegistration>(
        "user", "password", "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj", "agent_name", std::nullopt);
    agent = std::make_unique<AgentInfo>();

    MockHttpClient mockHttpClient;

    EXPECT_CALL(mockHttpClient, AuthenticateWithUserPassword(testing::_, testing::_, "user", "password"))
        .WillOnce(testing::Return(std::nullopt));

    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    const bool res = registration->Register(mockHttpClient);
    ASSERT_FALSE(res);
}

TEST_F(RegisterTest, RegistrationFailsIfServerResponseIsNotOk)
{
    AgentInfoPersistance agentInfoPersistance;
    agentInfoPersistance.ResetToDefault();

    registration = std::make_unique<agent_registration::AgentRegistration>(
        "user", "password", "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj", "agent_name", std::nullopt);
    agent = std::make_unique<AgentInfo>();

    MockHttpClient mockHttpClient;

    EXPECT_CALL(mockHttpClient, AuthenticateWithUserPassword(testing::_, testing::_, "user", "password"))
        .WillOnce(testing::Return("token"));

    boost::beast::http::response<boost::beast::http::dynamic_body> expectedResponse;
    expectedResponse.result(boost::beast::http::status::bad_request);

    EXPECT_CALL(mockHttpClient, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    const bool res = registration->Register(mockHttpClient);
    ASSERT_FALSE(res);
}

TEST_F(RegisterTest, RegistrationTestSuccessWithEmptyKey)
{
    AgentInfoPersistance agentInfoPersistance;
    agentInfoPersistance.ResetToDefault();

    registration =
        std::make_unique<agent_registration::AgentRegistration>("user", "password", "", "agent_name", std::nullopt);
    SysInfo sysInfo;
    agent = std::make_unique<AgentInfo>([&sysInfo]() mutable { return sysInfo.os(); },
                                        [&sysInfo]() mutable { return sysInfo.networks(); });

    MockHttpClient mockHttpClient;

    EXPECT_CALL(mockHttpClient, AuthenticateWithUserPassword(testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return("token"));

    const auto bodyJson = agent->GetMetadataInfo(true);

    http_client::HttpRequestParams reqParams(boost::beast::http::verb::post,
                                             "https://localhost:55000",
                                             "/agents",
                                             agent->GetHeaderInfo(),
                                             "token",
                                             "",
                                             bodyJson);

    boost::beast::http::response<boost::beast::http::dynamic_body> expectedResponse;
    expectedResponse.result(boost::beast::http::status::ok);

    EXPECT_CALL(mockHttpClient, PerformHttpRequest(testing::Eq(reqParams))).WillOnce(testing::Return(expectedResponse));

    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    const bool res = registration->Register(mockHttpClient);
    ASSERT_TRUE(res);
}

TEST_F(RegisterTest, RegistrationTestFailWithBadKey)
{
    ASSERT_THROW(agent_registration::AgentRegistration("user", "password", "badKey", "agent_name", std::nullopt),
                 std::invalid_argument);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
