#include <register.hpp>

#include <agent_info.hpp>

#include <gmock/gmock.h>
#include <gtest/gtest.h>

namespace beast = boost::beast;
namespace http = beast::http;
using ::testing::_;
using ::testing::MockFunction;
using ::testing::Return;

class RegisterTest : public ::testing::Test
{
protected:
    std::unique_ptr<AgentInfo> agent;

    MockFunction<http_client::AuthenticateFunctionType> MockAuthenticateFunction;
    MockFunction<registration::RegisterFunctionType> MockRegistrationFunction;

    void SetUp() override
    {
        agent = std::make_unique<AgentInfo>("agent_name", "agent_key", "fbdea7a6-0e0d-4d2c-8af3-9330ea8799a3");
    }
};

TEST_F(RegisterTest, RegistrationTest)
{
    const registration::UserCredentials userCredentials {"user", "password"};

    const std::optional<std::string> name = agent->GetName();

    EXPECT_CALL(MockAuthenticateFunction, Call("localhost", "55000", "user", "password")).WillOnce(Return("token"));
    EXPECT_CALL(MockRegistrationFunction, Call("localhost", "55000", "token", agent->GetUUID(), agent->GetKey(), name))
        .WillOnce(Return(http::status::ok));
    const bool res = registration::RegisterAgent(
        userCredentials, MockAuthenticateFunction.AsStdFunction(), MockRegistrationFunction.AsStdFunction());
    ASSERT_TRUE(res);
}

TEST_F(RegisterTest, RegistrationTestFail)
{
    const registration::UserCredentials userCredentials {"user", "password"};

    const std::optional<std::string> name = agent->GetName();

    EXPECT_CALL(MockAuthenticateFunction, Call("localhost", "55000", "user", "password")).WillOnce(Return("token"));
    EXPECT_CALL(MockRegistrationFunction, Call("localhost", "55000", "token", agent->GetUUID(), agent->GetKey(), name))
        .WillOnce(Return(http::status::bad_gateway));
    const bool res = registration::RegisterAgent(
        userCredentials, MockAuthenticateFunction.AsStdFunction(), MockRegistrationFunction.AsStdFunction());
    ASSERT_FALSE(res);
}

TEST_F(RegisterTest, AuthenticationFailDuringRegistrationTest)
{
    const registration::UserCredentials userCredentials {"user", "password"};

    EXPECT_CALL(MockAuthenticateFunction, Call("localhost", "55000", "user", "password"))
        .WillOnce(Return(std::nullopt));

    const bool res = registration::RegisterAgent(
        userCredentials, MockAuthenticateFunction.AsStdFunction(), MockRegistrationFunction.AsStdFunction());
    ASSERT_FALSE(res);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
