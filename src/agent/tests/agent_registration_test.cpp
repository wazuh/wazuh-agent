#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <agent_info.hpp>
#include <agent_info_persistance.hpp>
#include <agent_registration.hpp>
#include <ihttp_client.hpp>

#include "../http_client/tests/mocks/mock_http_client.hpp"
#include <mocks_persistence.hpp>

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

#include <memory>
#include <optional>
#include <string>

class RegisterTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        auto mockPersistencePtr = std::make_unique<MockPersistence>();
        mockPersistence = mockPersistencePtr.get();

        SetConstructorPersistenceExpectCalls();

        SysInfo sysInfo;
        agent = std::make_unique<AgentInfo>(
            ".",
            [sysInfo]() mutable { return sysInfo.os(); },
            [sysInfo]() mutable { return sysInfo.networks(); },
            true,
            std::make_shared<AgentInfoPersistance>("db_path", std::move(mockPersistencePtr)));

        agent->SetKey("4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj");
        agent->SetName("agent_name");
    }

    void SetConstructorPersistenceExpectCalls()
    {
        EXPECT_CALL(*mockPersistence, TableExists("agent_info")).WillOnce(testing::Return(true));
        EXPECT_CALL(*mockPersistence, TableExists("agent_group")).WillOnce(testing::Return(true));
        EXPECT_CALL(*mockPersistence, GetCount("agent_info", testing::_, testing::_))
            .WillOnce(testing::Return(0))
            .WillOnce(testing::Return(0));
        EXPECT_CALL(*mockPersistence, Insert("agent_info", testing::_)).Times(1);
    }

    void SetAgentInfoSaveExpectCalls()
    {
        // Mock for: m_persistence->ResetToDefault();
        EXPECT_CALL(*mockPersistence, DropTable("agent_info")).Times(1);
        EXPECT_CALL(*mockPersistence, DropTable("agent_group")).Times(1);
        EXPECT_CALL(*mockPersistence, CreateTable(testing::_, testing::_)).Times(2);
        EXPECT_CALL(*mockPersistence, GetCount("agent_info", testing::_, testing::_)).WillOnce(testing::Return(0));
        EXPECT_CALL(*mockPersistence, Insert(testing::_, testing::_)).Times(1);

        // Mock for: m_persistence->SetName(m_name); m_persistence->SetKey(m_key); m_persistence->SetUUID(m_uuid);
        EXPECT_CALL(*mockPersistence, Update("agent_info", testing::_, testing::_, testing::_)).Times(3);

        // Mock for: m_persistence->SetGroups(m_groups);
        EXPECT_CALL(*mockPersistence, BeginTransaction()).Times(1);
        EXPECT_CALL(*mockPersistence, Remove("agent_group", testing::_, testing::_)).Times(1);
        EXPECT_CALL(*mockPersistence, CommitTransaction(testing::_)).Times(1);
    }

    std::unique_ptr<AgentInfo> agent;
    std::unique_ptr<agent_registration::AgentRegistration> registration;
    MockPersistence* mockPersistence = nullptr;
};

TEST_F(RegisterTest, RegistrationTestSuccess)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    registration = std::make_unique<agent_registration::AgentRegistration>(std::move(mockHttpClient),
                                                                           "https://localhost:55000",
                                                                           "user",
                                                                           "password",
                                                                           "",
                                                                           "",
                                                                           ".",
                                                                           "full",
                                                                           std::move(*agent));

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::tuple<int, std::string> expectedResponse1 {200, R"({"data":{"token":"token"}})"};

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::tuple<int, std::string> expectedResponse2 {201, ""};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_))
        .Times(2)
        .WillOnce(testing::Return(expectedResponse1))
        .WillOnce(testing::Return(expectedResponse2));

    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)

    SetAgentInfoSaveExpectCalls();

    const bool res = registration->Register();
    ASSERT_TRUE(res);
}

TEST_F(RegisterTest, RegistrationFailsIfAuthenticationFails)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    registration = std::make_unique<agent_registration::AgentRegistration>(std::move(mockHttpClient),
                                                                           "https://localhost:55000",
                                                                           "user",
                                                                           "password",
                                                                           "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj",
                                                                           "agent_name",
                                                                           ".",
                                                                           "certificate",
                                                                           std::move(*agent));

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::tuple<int, std::string> expectedResponse {401, ""};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)
    const bool res = registration->Register();
    ASSERT_FALSE(res);
}

TEST_F(RegisterTest, RegistrationFailsIfServerResponseIsNotOk)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    registration = std::make_unique<agent_registration::AgentRegistration>(std::move(mockHttpClient),
                                                                           "https://localhost:55000",
                                                                           "user",
                                                                           "password",
                                                                           "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj",
                                                                           "agent_name",
                                                                           ".",
                                                                           "none",
                                                                           std::move(*agent));

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::tuple<int, std::string> expectedResponse1 {200, R"({"data":{"token":"token"}})"};

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::tuple<int, std::string> expectedResponse2 {400, ""};

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
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    registration = std::make_unique<agent_registration::AgentRegistration>(std::move(mockHttpClient),
                                                                           "https://localhost:55000",
                                                                           "user",
                                                                           "password",
                                                                           "",
                                                                           "agent_name",
                                                                           ".",
                                                                           "full",
                                                                           std::move(*agent));

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::tuple<int, std::string> expectedResponse1 {200, R"({"data":{"token":"token"}})"};

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::tuple<int, std::string> expectedResponse2 {201, ""};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_))
        .Times(2)
        .WillOnce(testing::Return(expectedResponse1))
        .WillOnce(testing::Return(expectedResponse2));

    // NOLINTNEXTLINE(cppcoreguidelines-init-variables)

    // Mock for: m_persistence->ResetToDefault();
    EXPECT_CALL(*mockPersistence, DropTable("agent_info")).Times(1);
    EXPECT_CALL(*mockPersistence, DropTable("agent_group")).Times(1);
    EXPECT_CALL(*mockPersistence, CreateTable(testing::_, testing::_)).Times(2);
    EXPECT_CALL(*mockPersistence, GetCount("agent_info", testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_CALL(*mockPersistence, Insert(testing::_, testing::_)).Times(1);

    // Mock for: m_persistence->SetName(m_name); m_persistence->SetKey(m_key); m_persistence->SetUUID(m_uuid);
    testing::InSequence seq;
    EXPECT_CALL(*mockPersistence,
                Update(testing::Eq("agent_info"),
                       testing::AllOf(
                           testing::SizeIs(1),
                           testing::Contains(testing::AllOf(testing::Field(&column::ColumnValue::Value, "agent_name"),
                                                            testing::Field(&column::ColumnName::Name, "name")))),
                       testing::_,
                       testing::_))
        .Times(1);

    EXPECT_CALL(*mockPersistence,
                Update(testing::Eq("agent_info"),
                       testing::AllOf(testing::SizeIs(1),
                                      testing::Contains(testing::AllOf(
                                          testing::Field(&column::ColumnValue::Value, testing::Not(testing::Eq(""))),
                                          testing::Field(&column::ColumnName::Name, "key")))),
                       testing::_,
                       testing::_))
        .Times(1);

    EXPECT_CALL(*mockPersistence,
                Update(testing::Eq("agent_info"),
                       testing::AllOf(testing::SizeIs(1),
                                      testing::Contains(testing::AllOf(
                                          testing::Field(&column::ColumnValue::Value, testing::Not(testing::Eq(""))),
                                          testing::Field(&column::ColumnName::Name, "uuid")))),
                       testing::_,
                       testing::_))
        .Times(1);

    // Mock for: m_persistence->SetGroups(m_groups);
    EXPECT_CALL(*mockPersistence, BeginTransaction()).Times(1);
    EXPECT_CALL(*mockPersistence, Remove("agent_group", testing::_, testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, CommitTransaction(testing::_)).Times(1);

    const bool res = registration->Register();
    ASSERT_TRUE(res);
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
                                                       "full",
                                                       std::move(*agent)),
                 std::invalid_argument);
}

TEST_F(RegisterTest, RegistrationTestFailWithHttpClientError)
{
    ASSERT_THROW(
        agent_registration::AgentRegistration(
            nullptr, "https://localhost:55000", "user", "password", "", "agent_name", ".", "full", std::move(*agent)),
        std::runtime_error);
}

TEST_F(RegisterTest, AuthenticateWithUserPassword_Success)
{
    auto mockHttpClient = std::make_unique<MockHttpClient>();
    auto mockHttpClientPtr = mockHttpClient.get();

    registration = std::make_unique<agent_registration::AgentRegistration>(std::move(mockHttpClient),
                                                                           "https://localhost:55000",
                                                                           "user",
                                                                           "password",
                                                                           "",
                                                                           "",
                                                                           ".",
                                                                           "full",
                                                                           std::move(*agent));

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::tuple<int, std::string> expectedResponse {200, R"({"data":{"token":"valid_token"}})"};

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

    registration = std::make_unique<agent_registration::AgentRegistration>(std::move(mockHttpClient),
                                                                           "https://localhost:55000",
                                                                           "user",
                                                                           "password",
                                                                           "",
                                                                           "",
                                                                           ".",
                                                                           "full",
                                                                           std::move(*agent));

    // NOLINTNEXTLINE(cppcoreguidelines-avoid-magic-numbers)
    std::tuple<int, std::string> expectedResponse {401, ""};

    EXPECT_CALL(*mockHttpClientPtr, PerformHttpRequest(testing::_)).WillOnce(testing::Return(expectedResponse));

    const auto token = registration->AuthenticateWithUserPassword();

    EXPECT_FALSE(token.has_value());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
