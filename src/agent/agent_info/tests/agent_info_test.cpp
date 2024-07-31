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
    EXPECT_EQ(agentInfo.GetIP(), "");
    EXPECT_EQ(agentInfo.GetUUID(), "");
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

TEST_F(AgentInfoTest, TestSetIP)
{
    AgentInfo agentInfo;
    const std::string newIP = "192.168.1.1";

    agentInfo.SetIP(newIP);
    EXPECT_EQ(agentInfo.GetIP(), newIP);

    const AgentInfo agentInfoReloaded;
    EXPECT_EQ(agentInfoReloaded.GetIP(), newIP);
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
