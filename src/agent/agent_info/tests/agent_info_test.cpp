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
    EXPECT_NE(agentInfo.GetUUID(), "");
}

TEST_F(AgentInfoTest, TestParameterizedConstructor)
{
    const std::string name = "new_name";
    const std::string ip = "192.168.1.1";
    const std::string uuid = "new_uuid";

    const AgentInfo agentInfo(name, ip, uuid);
    EXPECT_EQ(agentInfo.GetName(), name);
    EXPECT_EQ(agentInfo.GetIP(), ip);
    EXPECT_EQ(agentInfo.GetUUID(), uuid);
}

TEST_F(AgentInfoTest, TestPersistedValues)
{
    const AgentInfo agentInfo("test_name", "test_ip", "test_uuid");
    const AgentInfo agentInfoReloaded;
    EXPECT_EQ(agentInfoReloaded.GetName(), "test_name");
    EXPECT_EQ(agentInfoReloaded.GetIP(), "test_ip");
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
