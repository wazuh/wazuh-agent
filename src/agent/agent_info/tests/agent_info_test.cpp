#include <gtest/gtest.h>

#include <agent_info.hpp>
#include <agent_info_persistance.hpp>

#include <string>
#include <vector>

class AgentInfoTest : public ::testing::Test
{
protected:
    void SetUp() override
    {
        // We need to reset the database to the default state before each test
        AgentInfoPersistance agentInfoPersistance;
        agentInfoPersistance.ResetToDefault();
    }
};

TEST_F(AgentInfoTest, TestDefaultConstructor)
{
    EXPECT_NO_THROW(AgentInfo {});
}

TEST_F(AgentInfoTest, TestDefaultConstructorDefaultValues)
{
    const AgentInfo agentInfo;
    EXPECT_EQ(agentInfo.GetName(), "");
    EXPECT_EQ(agentInfo.GetKey(), "");
    EXPECT_NE(agentInfo.GetUUID(), "");
}

TEST_F(AgentInfoTest, TestPersistedValues)
{
    AgentInfo agentInfo;
    agentInfo.SetName("test_name");
    agentInfo.SetKey("4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj");
    agentInfo.SetUUID("test_uuid");
    const AgentInfo agentInfoReloaded;
    EXPECT_EQ(agentInfoReloaded.GetName(), "test_name");
    EXPECT_EQ(agentInfoReloaded.GetKey(), "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj");
    EXPECT_EQ(agentInfoReloaded.GetUUID(), "test_uuid");
}

TEST_F(AgentInfoTest, TestSetName)
{
    AgentInfo agentInfo;
    const std::string newName = "new_name";

    agentInfo.SetName(newName);
    EXPECT_EQ(agentInfo.GetName(), newName);

    const AgentInfo agentInfoReloaded;
    EXPECT_EQ(agentInfoReloaded.GetName(), newName);
}

TEST_F(AgentInfoTest, TestSetKey)
{
    AgentInfo agentInfo;
    const std::string newKey = "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj";

    agentInfo.SetKey(newKey);
    EXPECT_EQ(agentInfo.GetKey(), newKey);

    const AgentInfo agentInfoReloaded;
    EXPECT_EQ(agentInfoReloaded.GetKey(), newKey);
}

TEST_F(AgentInfoTest, TestSetBadKey)
{
    AgentInfo agentInfo;
    const std::string newKey1 = "4GhT7uFm";
    const std::string newKey2 = "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrN=";

    ASSERT_FALSE(agentInfo.SetKey(newKey1));
    ASSERT_FALSE(agentInfo.SetKey(newKey2));
}

TEST_F(AgentInfoTest, TestSetEmptyKey)
{
    AgentInfo agentInfo;
    const std::string newKey;

    agentInfo.SetKey(newKey);
    EXPECT_NE(agentInfo.GetKey(), newKey);

    const AgentInfo agentInfoReloaded;
    EXPECT_NE(agentInfoReloaded.GetKey(), newKey);
}

TEST_F(AgentInfoTest, TestSetUUID)
{
    AgentInfo agentInfo;
    const std::string newUUID = "new_uuid";

    agentInfo.SetUUID(newUUID);
    EXPECT_EQ(agentInfo.GetUUID(), newUUID);

    const AgentInfo agentInfoReloaded;
    EXPECT_EQ(agentInfoReloaded.GetUUID(), newUUID);
}

TEST_F(AgentInfoTest, TestSetGroups)
{
    AgentInfo agentInfo;
    const std::vector<std::string> newGroups = {"t_group_1", "t_group_2"};

    agentInfo.SetGroups(newGroups);
    EXPECT_EQ(agentInfo.GetGroups(), newGroups);

    const AgentInfo agentInfoReloaded;
    EXPECT_EQ(agentInfoReloaded.GetGroups(), newGroups);
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
