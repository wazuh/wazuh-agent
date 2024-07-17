#include <agent.hpp>

#include <gtest/gtest.h>

TEST(AgentTests, AgentConstruction)
{
    EXPECT_NO_THROW(Agent {});
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
