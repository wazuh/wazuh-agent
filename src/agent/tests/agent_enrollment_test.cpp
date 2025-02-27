#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <agent_enrollment.hpp>
#include <http_request_params.hpp>
#include <ihttp_client.hpp>
#include <mock_agent_info.hpp>
#include <mock_http_client.hpp>

#include <memory>
#include <string>

namespace
{
    const std::string AGENT_NAME = "agent_name";
    const std::string AGENT_KEY = "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj";
} // namespace

class EnrollmentTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        m_mockAgentInfo = std::make_unique<MockAgentInfo>();
        m_mockAgentInfoPtr = m_mockAgentInfo.get();

        m_mockHttpClient = std::make_unique<MockHttpClient>();
        m_mockHttpClientPtr = m_mockHttpClient.get();
    }

    std::unique_ptr<MockAgentInfo> m_mockAgentInfo;
    MockAgentInfo* m_mockAgentInfoPtr = nullptr;
    std::unique_ptr<MockHttpClient> m_mockHttpClient;
    MockHttpClient* m_mockHttpClientPtr = nullptr;
    std::unique_ptr<agent_enrollment::AgentEnrollment> m_enrollment;
};

TEST_F(EnrollmentTest, EnrollmentTestSuccess)
{
    EXPECT_CALL(*m_mockAgentInfoPtr, SetKey(testing::_)).WillOnce(testing::Return(true));
    EXPECT_CALL(*m_mockAgentInfoPtr, SetName(testing::_)).WillOnce(testing::Return(true));

    m_enrollment = std::make_unique<agent_enrollment::AgentEnrollment>(std::move(m_mockHttpClient),
                                                                       "https://localhost:55000",
                                                                       "user",
                                                                       "password",
                                                                       "",
                                                                       "",
                                                                       ".",
                                                                       "full",
                                                                       std::move(m_mockAgentInfo));

    const std::tuple<int, std::string> expectedResponse1 {http_client::HTTP_CODE_OK, R"({"data":{"token":"token"}})"};
    const std::tuple<int, std::string> expectedResponse2 {http_client::HTTP_CODE_CREATED, ""};

    EXPECT_CALL(*m_mockAgentInfoPtr, GetHeaderInfo()).Times(2).WillRepeatedly(testing::Return("header_info"));
    EXPECT_CALL(*m_mockAgentInfoPtr, GetMetadataInfo()).WillOnce(testing::Return("metadata_info"));

    EXPECT_CALL(*m_mockHttpClientPtr, PerformHttpRequest(testing::_))
        .Times(2)
        .WillOnce(testing::Return(expectedResponse1))
        .WillOnce(testing::Return(expectedResponse2));

    EXPECT_CALL(*m_mockAgentInfoPtr, Save()).Times(1);

    const bool res = m_enrollment->Enroll();
    ASSERT_TRUE(res);
}

TEST_F(EnrollmentTest, EnrollmentFailsIfAuthenticationFails)
{
    EXPECT_CALL(*m_mockAgentInfoPtr, SetKey(AGENT_KEY)).WillOnce(testing::Return(true));
    EXPECT_CALL(*m_mockAgentInfoPtr, SetName(AGENT_NAME)).WillOnce(testing::Return(true));

    m_enrollment = std::make_unique<agent_enrollment::AgentEnrollment>(std::move(m_mockHttpClient),
                                                                       "https://localhost:55000",
                                                                       "user",
                                                                       "password",
                                                                       AGENT_KEY,
                                                                       AGENT_NAME,
                                                                       ".",
                                                                       "certificate",
                                                                       std::move(m_mockAgentInfo));

    EXPECT_CALL(*m_mockAgentInfoPtr, GetHeaderInfo()).WillOnce(testing::Return("header_info"));

    const std::tuple<int, std::string> expectedResponse {http_client::HTTP_CODE_UNAUTHORIZED, ""};

    EXPECT_CALL(*m_mockHttpClientPtr, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    const bool res = m_enrollment->Enroll();
    ASSERT_FALSE(res);
}

TEST_F(EnrollmentTest, EnrollmentFailsIfServerResponseIsNotOk)
{
    EXPECT_CALL(*m_mockAgentInfoPtr, SetKey(AGENT_KEY)).WillOnce(testing::Return(true));
    EXPECT_CALL(*m_mockAgentInfoPtr, SetName(AGENT_NAME)).WillOnce(testing::Return(true));

    m_enrollment = std::make_unique<agent_enrollment::AgentEnrollment>(std::move(m_mockHttpClient),
                                                                       "https://localhost:55000",
                                                                       "user",
                                                                       "password",
                                                                       AGENT_KEY,
                                                                       AGENT_NAME,
                                                                       ".",
                                                                       "none",
                                                                       std::move(m_mockAgentInfo));

    EXPECT_CALL(*m_mockAgentInfoPtr, GetHeaderInfo()).Times(2).WillRepeatedly(testing::Return("header_info"));
    EXPECT_CALL(*m_mockAgentInfoPtr, GetMetadataInfo()).WillOnce(testing::Return("metadata_info"));

    const std::tuple<int, std::string> expectedResponse1 {http_client::HTTP_CODE_OK, R"({"data":{"token":"token"}})"};
    const std::tuple<int, std::string> expectedResponse2 {http_client::HTTP_CODE_BAD_REQUEST, ""};

    EXPECT_CALL(*m_mockHttpClientPtr, PerformHttpRequest(testing::_))
        .Times(2)
        .WillOnce(testing::Return(expectedResponse1))
        .WillOnce(testing::Return(expectedResponse2));

    const bool res = m_enrollment->Enroll();
    ASSERT_FALSE(res);
}

TEST_F(EnrollmentTest, EnrollmentWithoutAKeyGeneratesOneAutomatically)
{
    EXPECT_CALL(*m_mockAgentInfoPtr, SetKey("")).WillOnce(testing::Return(true));
    EXPECT_CALL(*m_mockAgentInfoPtr, SetName(AGENT_NAME)).WillOnce(testing::Return(true));

    m_enrollment = std::make_unique<agent_enrollment::AgentEnrollment>(std::move(m_mockHttpClient),
                                                                       "https://localhost:55000",
                                                                       "user",
                                                                       "password",
                                                                       "",
                                                                       AGENT_NAME,
                                                                       ".",
                                                                       "full",
                                                                       std::move(m_mockAgentInfo));

    EXPECT_CALL(*m_mockAgentInfoPtr, GetHeaderInfo()).Times(2).WillRepeatedly(testing::Return("header_info"));
    EXPECT_CALL(*m_mockAgentInfoPtr, GetMetadataInfo()).WillOnce(testing::Return("metadata_info"));

    const std::tuple<int, std::string> expectedResponse1 {http_client::HTTP_CODE_OK, R"({"data":{"token":"token"}})"};
    const std::tuple<int, std::string> expectedResponse2 {http_client::HTTP_CODE_CREATED, ""};

    EXPECT_CALL(*m_mockHttpClientPtr, PerformHttpRequest(testing::_))
        .Times(2)
        .WillOnce(testing::Return(expectedResponse1))
        .WillOnce(testing::Return(expectedResponse2));

    EXPECT_CALL(*m_mockAgentInfoPtr, Save()).Times(1);

    const bool res = m_enrollment->Enroll();
    ASSERT_TRUE(res);
}

TEST_F(EnrollmentTest, EnrollmentTestFailWithBadKey)
{
    EXPECT_CALL(*m_mockAgentInfoPtr, SetKey("badKey")).WillOnce(testing::Return(false));

    ASSERT_THROW(agent_enrollment::AgentEnrollment(std::make_unique<MockHttpClient>(),
                                                   "https://localhost:55000",
                                                   "user",
                                                   "password",
                                                   "badKey",
                                                   "agent_name",
                                                   ".",
                                                   "full",
                                                   std::move(m_mockAgentInfo)),
                 std::invalid_argument);
}

TEST_F(EnrollmentTest, EnrollmentTestFailWithHttpClientError)
{
    ASSERT_THROW(agent_enrollment::AgentEnrollment(nullptr,
                                                   "https://localhost:55000",
                                                   "user",
                                                   "password",
                                                   "",
                                                   "agent_name",
                                                   ".",
                                                   "full",
                                                   std::move(m_mockAgentInfo)),
                 std::runtime_error);
}

TEST_F(EnrollmentTest, AuthenticateWithUserPassword_Success)
{
    EXPECT_CALL(*m_mockAgentInfoPtr, SetKey("")).WillOnce(testing::Return(true));
    EXPECT_CALL(*m_mockAgentInfoPtr, SetName("")).WillOnce(testing::Return(true));

    m_enrollment = std::make_unique<agent_enrollment::AgentEnrollment>(std::move(m_mockHttpClient),
                                                                       "https://localhost:55000",
                                                                       "user",
                                                                       "password",
                                                                       "",
                                                                       "",
                                                                       ".",
                                                                       "full",
                                                                       std::move(m_mockAgentInfo));

    EXPECT_CALL(*m_mockAgentInfoPtr, GetHeaderInfo()).WillOnce(testing::Return("header_info"));

    const std::tuple<int, std::string> expectedResponse {http_client::HTTP_CODE_OK,
                                                         R"({"data":{"token":"valid_token"}})"};

    EXPECT_CALL(*m_mockHttpClientPtr, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    const auto token = m_enrollment->AuthenticateWithUserPassword();

    ASSERT_TRUE(token.has_value());

    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(token.value(), "valid_token");
}

TEST_F(EnrollmentTest, AuthenticateWithUserPassword_Failure)
{
    EXPECT_CALL(*m_mockAgentInfoPtr, SetKey("")).WillOnce(testing::Return(true));
    EXPECT_CALL(*m_mockAgentInfoPtr, SetName("")).WillOnce(testing::Return(true));

    m_enrollment = std::make_unique<agent_enrollment::AgentEnrollment>(std::move(m_mockHttpClient),
                                                                       "https://localhost:55000",
                                                                       "user",
                                                                       "password",
                                                                       "",
                                                                       "",
                                                                       ".",
                                                                       "full",
                                                                       std::move(m_mockAgentInfo));

    EXPECT_CALL(*m_mockAgentInfoPtr, GetHeaderInfo()).WillOnce(testing::Return("header_info"));

    const std::tuple<int, std::string> expectedResponse {http_client::HTTP_CODE_UNAUTHORIZED, ""};

    EXPECT_CALL(*m_mockHttpClientPtr, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    const auto token = m_enrollment->AuthenticateWithUserPassword();

    EXPECT_FALSE(token.has_value());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
