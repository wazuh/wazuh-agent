#include <gtest/gtest.h>

#include <agent_info_persistance.hpp>

#include <memory>
#include <string>
#include <vector>

class AgentInfoPersistanceTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        persistance = std::make_unique<AgentInfoPersistance>(".");
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
    EXPECT_EQ(persistance->GetKey(), "");
    EXPECT_EQ(persistance->GetUUID(), "");
}

TEST_F(AgentInfoPersistanceTest, TestSetName)
{
    const std::string newName = "new_name";
    persistance->SetName(newName);
    EXPECT_EQ(persistance->GetName(), newName);
}

TEST_F(AgentInfoPersistanceTest, TestSetKey)
{
    const std::string newKey = "new_key";
    persistance->SetKey(newKey);
    EXPECT_EQ(persistance->GetKey(), newKey);
}

TEST_F(AgentInfoPersistanceTest, TestSetUUID)
{
    const std::string newUUID = "new_uuid";
    persistance->SetUUID(newUUID);
    EXPECT_EQ(persistance->GetUUID(), newUUID);
}

TEST_F(AgentInfoPersistanceTest, TestSetGroups)
{
    const std::vector<std::string> newGroups = {"group_1", "group_2"};
    persistance->SetGroups(newGroups);
    EXPECT_EQ(persistance->GetGroups(), newGroups);
}

TEST_F(AgentInfoPersistanceTest, TestSetGroupsDelete)
{
    const std::vector<std::string> oldGroups = {"group_1", "group_2"};
    const std::vector<std::string> newGroups = {"group_3", "group_4"};
    persistance->SetGroups(oldGroups);
    EXPECT_EQ(persistance->GetGroups(), oldGroups);
    persistance->SetGroups(newGroups);
    EXPECT_EQ(persistance->GetGroups(), newGroups);
}

TEST_F(AgentInfoPersistanceTest, TestResetToDefault)
{
    const std::string newName = "new_name";
    persistance->SetName(newName);
    EXPECT_EQ(persistance->GetName(), newName);

    persistance->ResetToDefault();
    EXPECT_EQ(persistance->GetName(), "");
    EXPECT_EQ(persistance->GetKey(), "");
    EXPECT_EQ(persistance->GetUUID(), "");
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
