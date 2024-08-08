#include <register.hpp>

#include <gtest/gtest.h>

TEST(RegistrationTest, RegistrationTest)
{
    const registration::UserCredentials userCredentials {"user", "123456"};
    const bool res = registration::RegisterAgent(userCredentials);
    ASSERT_TRUE(res);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
