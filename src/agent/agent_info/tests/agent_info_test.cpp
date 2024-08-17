#include <gtest/gtest.h>

#include <agent_info.hpp>
#include <agent_info_persistance.hpp>

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

TEST_F(AgentInfoTest, TestParameterizedConstructor)
{
    const std::string name = "new_name";
    const std::string key = "new_key";
    const std::string uuid = "new_uuid";

    const AgentInfo agentInfo(name, key, uuid);
    EXPECT_EQ(agentInfo.GetName(), name);
    EXPECT_EQ(agentInfo.GetKey(), key);
    EXPECT_EQ(agentInfo.GetUUID(), uuid);
}

TEST_F(AgentInfoTest, TestPersistedValues)
{
    const AgentInfo agentInfo("test_name", "test_key", "test_uuid");
    const AgentInfo agentInfoReloaded;
    EXPECT_EQ(agentInfoReloaded.GetName(), "test_name");
    EXPECT_EQ(agentInfoReloaded.GetKey(), "test_key");
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
    const std::string newKey = "new_key";

    agentInfo.SetKey(newKey);
    EXPECT_EQ(agentInfo.GetKey(), newKey);

    const AgentInfo agentInfoReloaded;
    EXPECT_EQ(agentInfoReloaded.GetKey(), newKey);
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

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
