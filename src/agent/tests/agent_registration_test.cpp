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

const std::vector<std::string> DIFFERENT_KEYS = {"id", "name", "key"};

// NOLINTNEXTLINE(cppcoreguidelines-avoid-const-or-ref-data-members)
MATCHER_P(HTTPRequestDifferentData, expected, "Check that both HTTPs differ only on UUIDs, names and keys")
{
    auto body = nlohmann::json::parse(arg.Body);
    auto expectedBody = nlohmann::json::parse(expected.Body);

    for (const auto& [key, value] : body.items())
    {
        if (std::find(DIFFERENT_KEYS.begin(), DIFFERENT_KEYS.end(), key) != DIFFERENT_KEYS.end())
        {
            if (value == expectedBody.at(key))
            {
                return false;
            }
        }
        else if (key == "groups" || value != expectedBody.at(key))
        {
            return false;
        }
    }

    return (arg.Method == expected.Method && arg.Host == expected.Host && arg.Port == expected.Port &&
            arg.Endpoint == expected.Endpoint && arg.User_agent == expected.User_agent &&
            arg.Verification_Mode == expected.Verification_Mode && arg.Token == expected.Token &&
            arg.User_pass == expected.User_pass && arg.Use_Https == expected.Use_Https);
}

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

    registration = std::make_unique<agent_registration::AgentRegistration>(
        "https://localhost:55000", "user", "password", "", "", ".", "full");

    MockHttpClient mockHttpClient;

    EXPECT_CALL(mockHttpClient,
                AuthenticateWithUserPassword(testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return("token"));

    const auto bodyJson = agent->GetMetadataInfo();

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

    EXPECT_CALL(mockHttpClient, PerformHttpRequest(HTTPRequestDifferentData(reqParams)))
        .WillOnce(testing::Return(expectedResponse));

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

    EXPECT_TRUE(agentInfoPersistance.GetKey().empty());

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

    EXPECT_FALSE(agentInfoPersistance.GetKey().empty());
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
