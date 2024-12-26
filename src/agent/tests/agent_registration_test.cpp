#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <agent_info.hpp>
#include <agent_info_persistance.hpp>
#include <agent_registration.hpp>
#include <ihttp_client.hpp>
#include <ihttp_socket.hpp>

#include "../communicator/tests/mocks/mock_http_client.hpp"

#include <boost/asio.hpp>
#include <boost/beast.hpp>
#include <nlohmann/json.hpp>

#include <memory>
#include <optional>
#include <string>

class RegisterTest : public ::testing::Test
{
protected:
    void SetUp() override {}

    std::unique_ptr<AgentInfo> agent;
    std::unique_ptr<agent_registration::AgentRegistration> registration;
};

TEST_F(RegisterTest, RegistrationTestSuccess)
{
    AgentInfoPersistance agentInfoPersistance(".");
    agentInfoPersistance.ResetToDefault();

    SysInfo sysInfo;
    agent = std::make_unique<AgentInfo>(
        ".", [&sysInfo]() mutable { return sysInfo.os(); }, [&sysInfo]() mutable { return sysInfo.networks(); });

    agent->SetKey("4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj");
    agent->SetName("agent_name");
    agent->Save();

    registration = std::make_unique<agent_registration::AgentRegistration>(
        "https://localhost:55000", "user", "password", agent->GetKey(), agent->GetName(), ".", "full");

    MockHttpClient mockHttpClient;

    EXPECT_CALL(mockHttpClient,
                AuthenticateWithUserPassword(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return("token"));

    const auto bodyJson = agent->GetMetadataInfo(true);

    http_client::HttpRequestParams reqParams(boost::beast::http::verb::post,
                                             "https://localhost:55000",
                                             "/agents",
                                             agent->GetHeaderInfo(),
                                             "full",
                                             "token",
                                             "",
                                             bodyJson);

    boost::beast::http::response<boost::beast::http::dynamic_body> expectedResponse;
    expectedResponse.result(boost::beast::http::status::created);

    EXPECT_CALL(mockHttpClient, PerformHttpRequest(testing::Eq(reqParams))).WillOnce(testing::Return(expectedResponse));

    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    const bool res = registration->Register(mockHttpClient);
    ASSERT_TRUE(res);
}

TEST_F(RegisterTest, RegistrationFailsIfAuthenticationFails)
{
    AgentInfoPersistance agentInfoPersistance(".");
    agentInfoPersistance.ResetToDefault();

    registration = std::make_unique<agent_registration::AgentRegistration>("https://localhost:55000",
                                                                           "user",
                                                                           "password",
                                                                           "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj",
                                                                           "agent_name",
                                                                           ".",
                                                                           "certificate");
    agent = std::make_unique<AgentInfo>(".");

    MockHttpClient mockHttpClient;

    EXPECT_CALL(mockHttpClient, AuthenticateWithUserPassword(testing::_, testing::_, "user", "password", testing::_))
        .WillOnce(testing::Return(std::nullopt));

    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    const bool res = registration->Register(mockHttpClient);
    ASSERT_FALSE(res);
}

TEST_F(RegisterTest, RegistrationFailsIfServerResponseIsNotOk)
{
    AgentInfoPersistance agentInfoPersistance(".");
    agentInfoPersistance.ResetToDefault();

    registration = std::make_unique<agent_registration::AgentRegistration>(
        "https://localhost:55000", "user", "password", "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj", "agent_name", ".", "none");
    agent = std::make_unique<AgentInfo>(".");

    MockHttpClient mockHttpClient;

    EXPECT_CALL(mockHttpClient, AuthenticateWithUserPassword(testing::_, testing::_, "user", "password", testing::_))
        .WillOnce(testing::Return("token"));

    boost::beast::http::response<boost::beast::http::dynamic_body> expectedResponse;
    expectedResponse.result(boost::beast::http::status::bad_request);

    EXPECT_CALL(mockHttpClient, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    const bool res = registration->Register(mockHttpClient);
    ASSERT_FALSE(res);
}

TEST_F(RegisterTest, RegisteringWithoutAKeyGeneratesOneAutomatically)
{
    AgentInfoPersistance agentInfoPersistance(".");
    agentInfoPersistance.ResetToDefault();

    agent = std::make_unique<AgentInfo>(".");
    EXPECT_TRUE(agent->GetKey().empty());

    registration = std::make_unique<agent_registration::AgentRegistration>(
        "https://localhost:55000", "user", "password", "", "agent_name", ".", "full");

    MockHttpClient mockHttpClient;

    EXPECT_CALL(mockHttpClient,
                AuthenticateWithUserPassword(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return("token"));

    boost::beast::http::response<boost::beast::http::dynamic_body> expectedResponse;
    expectedResponse.result(boost::beast::http::status::created);

    EXPECT_CALL(mockHttpClient, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    const bool res = registration->Register(mockHttpClient);
    ASSERT_TRUE(res);

    agent = std::make_unique<AgentInfo>(".");
    EXPECT_FALSE(agent->GetKey().empty());
}

TEST_F(RegisterTest, RegistrationTestFailWithBadKey)
{
    ASSERT_THROW(agent_registration::AgentRegistration(
                     "https://localhost:55000", "user", "password", "badKey", "agent_name", ".", "full"),
                 std::invalid_argument);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
