#include <gtest/gtest.h>

#include <agent_info_persistance.hpp>

#include <memory>
#include <string>

class AgentInfoPersistanceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        persistance = std::make_unique<AgentInfoPersistance>("agent_info_test.db");
    }

    std::unique_ptr<AgentInfoPersistance> persistance;
};

TEST_F(AgentInfoPersistanceTest, TestConstruction)
{
    EXPECT_NE(persistance, nullptr);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
