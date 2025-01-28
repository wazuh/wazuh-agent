#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <agent_info.hpp>
#include <agent_info_persistance.hpp>
#include <agent_registration.hpp>
#include <ihttp_client.hpp>
#include <ihttp_socket.hpp>

#include "../http_client/tests/mocks/mock_http_client.hpp"

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

#include <memory>
#include <optional>
#include <string>

class RegisterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        SysInfo sysInfo;
        agent = std::make_unique<AgentInfo>(
            ".",
            [&sysInfo]() mutable { return sysInfo.os(); },
            [&sysInfo]() mutable { return sysInfo.networks(); },
            true);

        agent->SetKey("4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj");
        agent->SetName("agent_name");
        agent->Save();
    }

    std::unique_ptr<AgentInfo> agent;
    std::unique_ptr<agent_registration::AgentRegistration> registration;
};

TEST_F(RegisterTest, RegistrationTestSuccess)
{
    AgentInfoPersistance agentInfoPersistance(".");
    agentInfoPersistance.ResetToDefault();

    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    registration = std::make_unique<agent_registration::AgentRegistration>(
        std::move(mockHttpClient), "https://localhost:55000", "user", "password", "", "", ".", "full");

    boost::beast::http::response<boost::beast::http::dynamic_body> expectedResponse1;
    expectedResponse1.result(boost::beast::http::status::ok);
    boost::beast::ostream(expectedResponse1.body()) << R"({"data":{"token":"token"}})";

    const auto bodyJson = agent->GetMetadataInfo();

    http_client::HttpRequestParams reqParams(boost::beast::http::verb::post,
                                             "https://localhost:55000",
                                             "/agents",
                                             agent->GetHeaderInfo(),
                                             "full",
                                             "token",
                                             "",
                                             bodyJson);

    boost::beast::http::response<boost::beast::http::dynamic_body> expectedResponse2;
    expectedResponse2.result(boost::beast::http::status::created);

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_))
        .Times(2)
        .WillOnce(testing::Return(expectedResponse1))
        .WillOnce(testing::Return(expectedResponse2));

    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    const bool res = registration->Register();
    ASSERT_TRUE(res);
}

TEST_F(RegisterTest, RegistrationFailsIfAuthenticationFails)
{
    AgentInfoPersistance agentInfoPersistance(".");
    agentInfoPersistance.ResetToDefault();

    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    registration = std::make_unique<agent_registration::AgentRegistration>(std::move(mockHttpClient),
                                                                           "https://localhost:55000",
                                                                           "user",
                                                                           "password",
                                                                           "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj",
                                                                           "agent_name",
                                                                           ".",
                                                                           "certificate");

    boost::beast::http::response<boost::beast::http::dynamic_body> expectedResponse;
    expectedResponse.result(boost::beast::http::status::unauthorized);

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    const bool res = registration->Register();
    ASSERT_FALSE(res);
}

TEST_F(RegisterTest, RegistrationFailsIfServerResponseIsNotOk)
{
    AgentInfoPersistance agentInfoPersistance(".");
    agentInfoPersistance.ResetToDefault();

    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    registration = std::make_unique<agent_registration::AgentRegistration>(std::move(mockHttpClient),
                                                                           "https://localhost:55000",
                                                                           "user",
                                                                           "password",
                                                                           "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj",
                                                                           "agent_name",
                                                                           ".",
                                                                           "none");

    boost::beast::http::response<boost::beast::http::dynamic_body> expectedResponse1;
    expectedResponse1.result(boost::beast::http::status::ok);
    boost::beast::ostream(expectedResponse1.body()) << R"({"data":{"token":"token"}})";

    boost::beast::http::response<boost::beast::http::dynamic_body> expectedResponse2;
    expectedResponse2.result(boost::beast::http::status::bad_request);

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_))
        .Times(2)
        .WillOnce(testing::Return(expectedResponse1))
        .WillOnce(testing::Return(expectedResponse2));

    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    const bool res = registration->Register();
    ASSERT_FALSE(res);
}

TEST_F(RegisterTest, RegisteringWithoutAKeyGeneratesOneAutomatically)
{
    AgentInfoPersistance agentInfoPersistance(".");
    agentInfoPersistance.ResetToDefault();

    EXPECT_TRUE(agentInfoPersistance.GetKey().empty());

    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    registration = std::make_unique<agent_registration::AgentRegistration>(
        std::move(mockHttpClient), "https://localhost:55000", "user", "password", "", "agent_name", ".", "full");

    boost::beast::http::response<boost::beast::http::dynamic_body> expectedResponse1;
    expectedResponse1.result(boost::beast::http::status::ok);
    boost::beast::ostream(expectedResponse1.body()) << R"({"data":{"token":"token"}})";

    boost::beast::http::response<boost::beast::http::dynamic_body> expectedResponse2;
    expectedResponse2.result(boost::beast::http::status::created);

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_))
        .Times(2)
        .WillOnce(testing::Return(expectedResponse1))
        .WillOnce(testing::Return(expectedResponse2));

    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    const bool res = registration->Register();
    ASSERT_TRUE(res);

    EXPECT_FALSE(agentInfoPersistance.GetKey().empty());
}

TEST_F(RegisterTest, RegistrationTestFailWithBadKey)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();

    ASSERT_THROW(agent_registration::AgentRegistration(std::move(mockHttpClient),
                                                       "https://localhost:55000",
                                                       "user",
                                                       "password",
                                                       "badKey",
                                                       "agent_name",
                                                       ".",
                                                       "full"),
                 std::invalid_argument);
}

TEST_F(RegisterTest, RegistrationTestFailWithHttpClientError)
{
    ASSERT_THROW(agent_registration::AgentRegistration(
                     nullptr, "https://localhost:55000", "user", "password", "", "agent_name", ".", "full"),
                 std::runtime_error);
}

TEST_F(RegisterTest, AuthenticateWithUserPassword_Success)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    registration = std::make_unique<agent_registration::AgentRegistration>(
        std::move(mockHttpClient), "https://localhost:55000", "user", "password", "", "", ".", "full");

    boost::beast::http::response<boost::beast::http::dynamic_body> expectedResponse;
    expectedResponse.result(boost::beast::http::status::ok);
    boost::beast::ostream(expectedResponse.body()) << R"({"data":{"token":"valid_token"}})";

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    const auto token = registration->AuthenticateWithUserPassword();

    ASSERT_TRUE(token.has_value());

    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(token.value(), "valid_token");
}

TEST_F(RegisterTest, AuthenticateWithUserPassword_Failure)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    registration = std::make_unique<agent_registration::AgentRegistration>(
        std::move(mockHttpClient), "https://localhost:55000", "user", "password", "", "", ".", "full");

    boost::beast::http::response<boost::beast::http::dynamic_body> expectedResponse;
    expectedResponse.result(boost::beast::http::status::unauthorized);

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    const auto token = registration->AuthenticateWithUserPassword();

    EXPECT_FALSE(token.has_value());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
