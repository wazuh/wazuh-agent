#include "communicator.hpp"
#include <gtest/gtest.h>

using namespace communicator;

TEST(HttpPostTest, AuthenticationTest)
{
    Communicator communicator;
    int res = communicator.SendAuthenticationRequest();
    ASSERT_EQ(res, 200);
}

TEST(HttpPostTest, RegistrationTest)
{
    Communicator communicator;
    communicator.SendAuthenticationRequest();
    int res = communicator.SendRegistrationRequest();
    ASSERT_EQ(res, 200);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
