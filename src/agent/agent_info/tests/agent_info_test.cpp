#include <gtest/gtest.h>

#include <agent_info.hpp>

class AgentInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
    }
};

TEST_F(AgentInfoTest, TestDefaultConstructor)
{
    EXPECT_NO_THROW(AgentInfo {});
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
