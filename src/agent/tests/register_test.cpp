#include <register.hpp>

#include <gtest/gtest.h>

TEST(RegistrationTest, RegistrationTest)
{
    const std::string user = "user";
    const std::string password = "123456";
    const std::optional<std::string> name = "name";
    const std::optional<std::string> ip = "192.168.56.1.2";
    const bool res = RegisterAgent(user, password, name, ip);
    ASSERT_TRUE(res);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
