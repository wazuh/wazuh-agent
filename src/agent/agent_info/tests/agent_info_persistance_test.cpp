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

TEST_F(AgentInfoPersistanceTest, TestSetName)
{
    const std::string newName = "new_name";
    persistance->SetName(newName);
    EXPECT_EQ(persistance->GetName(), newName);
}

TEST_F(AgentInfoPersistanceTest, TestSetIP)
{
    const std::string newIP = "192.168.1.1";
    persistance->SetIP(newIP);
    EXPECT_EQ(persistance->GetIP(), newIP);
}

TEST_F(AgentInfoPersistanceTest, TestSetUUID)
{
    const std::string newUUID = "new_uuid";
    persistance->SetUUID(newUUID);
    EXPECT_EQ(persistance->GetUUID(), newUUID);
}

TEST_F(AgentInfoPersistanceTest, TestResetToDefault)
{
    const std::string newName = "new_name";
    persistance->SetName(newName);
    EXPECT_EQ(persistance->GetName(), newName);

    persistance->ResetToDefault();
    EXPECT_EQ(persistance->GetName(), "");
    EXPECT_EQ(persistance->GetIP(), "");
    EXPECT_EQ(persistance->GetUUID(), "");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
