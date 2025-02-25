#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <agent_enrollment.hpp>
#include <agent_info.hpp>
#include <agent_info_persistance.hpp>
#include <http_request_params.hpp>
#include <ihttp_client.hpp>

#include "../http_client/tests/mocks/mock_http_client.hpp"
#include <mocks_persistence.hpp>

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

#include <memory>
#include <optional>
#include <string>

class EnrollmentTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto mockPersistencePtr = std::make_unique<MockPersistence>();
        m_mockPersistence = mockPersistencePtr.get();

        SetConstructorPersistenceExpectCalls();

        SysInfo sysInfo;
        m_agentInfo = std::make_unique<AgentInfo>(
            ".",
            [sysInfo]() mutable { return sysInfo.os(); },
            [sysInfo]() mutable { return sysInfo.networks(); },
            true,
            std::make_shared<AgentInfoPersistance>("db_path", std::move(mockPersistencePtr)));

        m_agentInfo->SetKey("4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj");
        m_agentInfo->SetName("agent_name");
    }

    void SetConstructorPersistenceExpectCalls()
    {
        EXPECT_CALL(*m_mockPersistence, TableExists("agent_info")).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockPersistence, TableExists("agent_group")).WillOnce(testing::Return(true));
        EXPECT_CALL(*m_mockPersistence, GetCount("agent_info", testing::_, testing::_))
            .WillOnce(testing::Return(0))
            .WillOnce(testing::Return(0));
        EXPECT_CALL(*m_mockPersistence, Insert("agent_info", testing::_)).Times(1);
    }

    void SetAgentInfoSaveExpectCalls()
    {
        // Mock for: m_persistence->ResetToDefault();
        EXPECT_CALL(*m_mockPersistence, DropTable("agent_info")).Times(1);
        EXPECT_CALL(*m_mockPersistence, DropTable("agent_group")).Times(1);
        EXPECT_CALL(*m_mockPersistence, CreateTable(testing::_, testing::_)).Times(2);
        EXPECT_CALL(*m_mockPersistence, GetCount("agent_info", testing::_, testing::_)).WillOnce(testing::Return(0));
        EXPECT_CALL(*m_mockPersistence, Insert(testing::_, testing::_)).Times(1);

        // Mock for: m_persistence->SetName(m_name); m_persistence->SetKey(m_key); m_persistence->SetUUID(m_uuid);
        EXPECT_CALL(*m_mockPersistence, Update("agent_info", testing::_, testing::_, testing::_)).Times(3);

        // Mock for: m_persistence->SetGroups(m_groups);
        EXPECT_CALL(*m_mockPersistence, BeginTransaction()).Times(1);
        EXPECT_CALL(*m_mockPersistence, Remove("agent_group", testing::_, testing::_)).Times(1);
        EXPECT_CALL(*m_mockPersistence, CommitTransaction(testing::_)).Times(1);
    }

    std::unique_ptr<AgentInfo> m_agentInfo;
    std::unique_ptr<agent_enrollment::AgentEnrollment> m_enrollment;
    MockPersistence* m_mockPersistence = nullptr;
};

TEST_F(EnrollmentTest, EnrollmentTestSuccess)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    const auto mockHttpClientPtr = mockHttpClient.get();

    m_enrollment = std::make_unique<agent_enrollment::AgentEnrollment>(std::move(mockHttpClient),
                                                                       "https://localhost:55000",
                                                                       "user",
                                                                       "password",
                                                                       "",
                                                                       "",
                                                                       ".",
                                                                       "full",
                                                                       std::move(*m_agentInfo));

    const std::tuple<int, std::string> expectedResponse1 {http_client::HTTP_CODE_OK, R"({"data":{"token":"token"}})"};
    const std::tuple<int, std::string> expectedResponse2 {http_client::HTTP_CODE_CREATED, ""};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_))
        .Times(2)
        .WillOnce(testing::Return(expectedResponse1))
        .WillOnce(testing::Return(expectedResponse2));

    SetAgentInfoSaveExpectCalls();

    const bool res = m_enrollment->Enroll();
    ASSERT_TRUE(res);
}

TEST_F(EnrollmentTest, EnrollmentFailsIfAuthenticationFails)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    const auto mockHttpClientPtr = mockHttpClient.get();

    m_enrollment = std::make_unique<agent_enrollment::AgentEnrollment>(std::move(mockHttpClient),
                                                                       "https://localhost:55000",
                                                                       "user",
                                                                       "password",
                                                                       "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj",
                                                                       "agent_name",
                                                                       ".",
                                                                       "certificate",
                                                                       std::move(*m_agentInfo));

    const std::tuple<int, std::string> expectedResponse {http_client::HTTP_CODE_UNAUTHORIZED, ""};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    const bool res = m_enrollment->Enroll();
    ASSERT_FALSE(res);
}

TEST_F(EnrollmentTest, EnrollmentFailsIfServerResponseIsNotOk)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    const auto mockHttpClientPtr = mockHttpClient.get();

    m_enrollment = std::make_unique<agent_enrollment::AgentEnrollment>(std::move(mockHttpClient),
                                                                       "https://localhost:55000",
                                                                       "user",
                                                                       "password",
                                                                       "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj",
                                                                       "agent_name",
                                                                       ".",
                                                                       "none",
                                                                       std::move(*m_agentInfo));

    const std::tuple<int, std::string> expectedResponse1 {http_client::HTTP_CODE_OK, R"({"data":{"token":"token"}})"};
    const std::tuple<int, std::string> expectedResponse2 {http_client::HTTP_CODE_BAD_REQUEST, ""};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_))
        .Times(2)
        .WillOnce(testing::Return(expectedResponse1))
        .WillOnce(testing::Return(expectedResponse2));

    const bool res = m_enrollment->Enroll();
    ASSERT_FALSE(res);
}

TEST_F(EnrollmentTest, EnrollmentWithoutAKeyGeneratesOneAutomatically)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    const auto mockHttpClientPtr = mockHttpClient.get();

    m_enrollment = std::make_unique<agent_enrollment::AgentEnrollment>(std::move(mockHttpClient),
                                                                       "https://localhost:55000",
                                                                       "user",
                                                                       "password",
                                                                       "",
                                                                       "agent_name",
                                                                       ".",
                                                                       "full",
                                                                       std::move(*m_agentInfo));

    const std::tuple<int, std::string> expectedResponse1 {http_client::HTTP_CODE_OK, R"({"data":{"token":"token"}})"};
    const std::tuple<int, std::string> expectedResponse2 {http_client::HTTP_CODE_CREATED, ""};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_))
        .Times(2)
        .WillOnce(testing::Return(expectedResponse1))
        .WillOnce(testing::Return(expectedResponse2));

    // Mock for: m_persistence->ResetToDefault();
    EXPECT_CALL(*m_mockPersistence, DropTable("agent_info")).Times(1);
    EXPECT_CALL(*m_mockPersistence, DropTable("agent_group")).Times(1);
    EXPECT_CALL(*m_mockPersistence, CreateTable(testing::_, testing::_)).Times(2);
    EXPECT_CALL(*m_mockPersistence, GetCount("agent_info", testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_CALL(*m_mockPersistence, Insert(testing::_, testing::_)).Times(1);

    // Mock for: m_persistence->SetName(m_name); m_persistence->SetKey(m_key); m_persistence->SetUUID(m_uuid);
    const testing::InSequence seq;
    EXPECT_CALL(*m_mockPersistence,
                Update(testing::Eq("agent_info"),
                       testing::AllOf(
                           testing::SizeIs(1),
                           testing::Contains(testing::AllOf(testing::Field(&column::ColumnValue::Value, "agent_name"),
                                                            testing::Field(&column::ColumnName::Name, "name")))),
                       testing::_,
                       testing::_))
        .Times(1);

    EXPECT_CALL(*m_mockPersistence,
                Update(testing::Eq("agent_info"),
                       testing::AllOf(testing::SizeIs(1),
                                      testing::Contains(testing::AllOf(
                                          testing::Field(&column::ColumnValue::Value, testing::Not(testing::Eq(""))),
                                          testing::Field(&column::ColumnName::Name, "key")))),
                       testing::_,
                       testing::_))
        .Times(1);

    EXPECT_CALL(*m_mockPersistence,
                Update(testing::Eq("agent_info"),
                       testing::AllOf(testing::SizeIs(1),
                                      testing::Contains(testing::AllOf(
                                          testing::Field(&column::ColumnValue::Value, testing::Not(testing::Eq(""))),
                                          testing::Field(&column::ColumnName::Name, "uuid")))),
                       testing::_,
                       testing::_))
        .Times(1);

    // Mock for: m_persistence->SetGroups(m_groups);
    EXPECT_CALL(*m_mockPersistence, BeginTransaction()).Times(1);
    EXPECT_CALL(*m_mockPersistence, Remove("agent_group", testing::_, testing::_)).Times(1);
    EXPECT_CALL(*m_mockPersistence, CommitTransaction(testing::_)).Times(1);

    const bool res = m_enrollment->Enroll();
    ASSERT_TRUE(res);
}

TEST_F(EnrollmentTest, EnrollmentTestFailWithBadKey)
{
    ASSERT_THROW(agent_enrollment::AgentEnrollment(std::make_unique<MockHttpClient>(),
                                                   "https://localhost:55000",
                                                   "user",
                                                   "password",
                                                   "badKey",
                                                   "agent_name",
                                                   ".",
                                                   "full",
                                                   std::move(*m_agentInfo)),
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
                                                   std::move(*m_agentInfo)),
                 std::runtime_error);
}

TEST_F(EnrollmentTest, AuthenticateWithUserPassword_Success)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    const auto mockHttpClientPtr = mockHttpClient.get();

    m_enrollment = std::make_unique<agent_enrollment::AgentEnrollment>(std::move(mockHttpClient),
                                                                       "https://localhost:55000",
                                                                       "user",
                                                                       "password",
                                                                       "",
                                                                       "",
                                                                       ".",
                                                                       "full",
                                                                       std::move(*m_agentInfo));

    const std::tuple<int, std::string> expectedResponse {http_client::HTTP_CODE_OK,
                                                         R"({"data":{"token":"valid_token"}})"};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    const auto token = m_enrollment->AuthenticateWithUserPassword();

    ASSERT_TRUE(token.has_value());

    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    EXPECT_EQ(token.value(), "valid_token");
}

TEST_F(EnrollmentTest, AuthenticateWithUserPassword_Failure)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    const auto mockHttpClientPtr = mockHttpClient.get();

    m_enrollment = std::make_unique<agent_enrollment::AgentEnrollment>(std::move(mockHttpClient),
                                                                       "https://localhost:55000",
                                                                       "user",
                                                                       "password",
                                                                       "",
                                                                       "",
                                                                       ".",
                                                                       "full",
                                                                       std::move(*m_agentInfo));

    const std::tuple<int, std::string> expectedResponse {http_client::HTTP_CODE_UNAUTHORIZED, ""};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    const auto token = m_enrollment->AuthenticateWithUserPassword();

    EXPECT_FALSE(token.has_value());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
