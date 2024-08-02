#include "communicator.hpp"
#include <gtest/gtest.h>

using namespace communicator;

TEST(CommunicatorTest, GetTokenRemainingSecsTest)
{
    Communicator communicator(nullptr);
    int res = communicator.GetTokenRemainingSecs();
    //ASSERT_EQ(res, 0);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
