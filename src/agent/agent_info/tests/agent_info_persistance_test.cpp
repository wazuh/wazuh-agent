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
        persistance->ResetToDefault();
    }

    std::unique_ptr<AgentInfoPersistance> persistance;
};

TEST_F(AgentInfoPersistanceTest, TestConstruction)
{
    EXPECT_NE(persistance, nullptr);
}

TEST_F(AgentInfoPersistanceTest, TestDefaultValues)
{
    EXPECT_EQ(persistance->GetName(), "");
    EXPECT_EQ(persistance->GetIP(), "");
    EXPECT_EQ(persistance->GetUUID(), "");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
