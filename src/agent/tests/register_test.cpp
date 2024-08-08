#include <register.hpp>

#include <gtest/gtest.h>

TEST(RegistrationTest, RegistrationTest)
{
    const registration::UserCredentials userCredentials {"user", "123456"};
    const registration::AgentInfoOptionalData agentInfoOptionalData {"name", "192.168.56.1.2"};
    const bool res = registration::RegisterAgent(userCredentials, agentInfoOptionalData);
    ASSERT_TRUE(res);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
