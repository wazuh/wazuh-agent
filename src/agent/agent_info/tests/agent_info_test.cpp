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
        AgentInfoPersistance agentInfoPersistance(".");
        agentInfoPersistance.ResetToDefault();
    }
};

TEST_F(AgentInfoTest, TestDefaultConstructorDefaultValues)
{
    EXPECT_NO_THROW({
        const AgentInfo agentInfo(".");
        EXPECT_EQ(agentInfo.GetName(), "");
        EXPECT_EQ(agentInfo.GetKey(), "");
        EXPECT_NE(agentInfo.GetUUID(), "");
    });
}

TEST_F(AgentInfoTest, TestPersistedValues)
{
    AgentInfo agentInfo(".");
    agentInfo.SetName("test_name");
    agentInfo.SetKey("4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj");
    agentInfo.SetUUID("test_uuid");
    agentInfo.Save();
    const AgentInfo agentInfoReloaded(".");
    EXPECT_EQ(agentInfoReloaded.GetName(), "test_name");
    EXPECT_EQ(agentInfoReloaded.GetKey(), "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj");
    EXPECT_EQ(agentInfoReloaded.GetUUID(), "test_uuid");
}

TEST_F(AgentInfoTest, TestSetName)
{
    AgentInfo agentInfo(".");
    const std::string oldName = agentInfo.GetName();
    const std::string newName = "new_name";

    agentInfo.SetName(newName);
    EXPECT_EQ(agentInfo.GetName(), newName);

    const AgentInfo agentInfoReloaded(".");
    EXPECT_EQ(agentInfoReloaded.GetName(), oldName);
}

TEST_F(AgentInfoTest, TestSetKey)
{
    AgentInfo agentInfo(".");
    const std::string oldKey = agentInfo.GetKey();
    const std::string newKey = "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj";

    agentInfo.SetKey(newKey);
    EXPECT_EQ(agentInfo.GetKey(), newKey);

    const AgentInfo agentInfoReloaded(".");
    EXPECT_EQ(agentInfoReloaded.GetKey(), oldKey);
}

TEST_F(AgentInfoTest, TestSetBadKey)
{
    AgentInfo agentInfo(".");
    const std::string newKey1 = "4GhT7uFm";
    const std::string newKey2 = "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrN=";

    ASSERT_FALSE(agentInfo.SetKey(newKey1));
    ASSERT_FALSE(agentInfo.SetKey(newKey2));
}

TEST_F(AgentInfoTest, TestSetEmptyKey)
{
    AgentInfo agentInfo(".");
    const std::string newKey;
    const std::string oldKey = agentInfo.GetKey();

    agentInfo.SetKey(newKey);
    EXPECT_NE(agentInfo.GetKey(), newKey);

    const AgentInfo agentInfoReloaded(".");
    EXPECT_EQ(agentInfoReloaded.GetKey(), oldKey);
}

TEST_F(AgentInfoTest, TestSetUUID)
{
    AgentInfo agentInfo(".");
    const std::string newUUID = "new_uuid";

    agentInfo.SetUUID(newUUID);
    EXPECT_EQ(agentInfo.GetUUID(), newUUID);

    const AgentInfo agentInfoReloaded(".");
    EXPECT_NE(agentInfoReloaded.GetUUID(), newUUID);
}

TEST_F(AgentInfoTest, TestSetGroups)
{
    AgentInfo agentInfo(".");
    const std::vector<std::string> oldGroups = agentInfo.GetGroups();
    const std::vector<std::string> newGroups = {"t_group_1", "t_group_2"};

    agentInfo.SetGroups(newGroups);
    EXPECT_EQ(agentInfo.GetGroups(), newGroups);

    const AgentInfo agentInfoReloaded(".");
    EXPECT_EQ(agentInfoReloaded.GetGroups(), oldGroups);
}

TEST_F(AgentInfoTest, TestSaveGroups)
{
    AgentInfo agentInfo(".");
    const std::vector<std::string> oldGroups = agentInfo.GetGroups();
    const std::vector<std::string> newGroups = {"t_group_1", "t_group_2"};

    agentInfo.SetGroups(newGroups);
    agentInfo.SaveGroups();
    EXPECT_EQ(agentInfo.GetGroups(), newGroups);

    const AgentInfo agentInfoReloaded(".");
    EXPECT_EQ(agentInfoReloaded.GetGroups(), newGroups);
}

TEST_F(AgentInfoTest, TestLoadMetadataInfoNoSysInfo)
{
    const AgentInfo agentInfo(".", nullptr, nullptr, true);

    auto metadataInfo = nlohmann::json::parse(agentInfo.GetMetadataInfo());

    EXPECT_TRUE(metadataInfo != nullptr);

    // Agent information
    EXPECT_EQ(metadataInfo["type"], agentInfo.GetType());
    EXPECT_EQ(metadataInfo["version"], agentInfo.GetVersion());
    EXPECT_EQ(metadataInfo["id"], agentInfo.GetUUID());
    EXPECT_EQ(metadataInfo["name"], agentInfo.GetName());
    EXPECT_EQ(metadataInfo["key"], agentInfo.GetKey());
    EXPECT_TRUE(metadataInfo["groups"] == nullptr);

    // Endpoint information
    EXPECT_TRUE(metadataInfo["host"] == nullptr);
}

TEST_F(AgentInfoTest, TestLoadMetadataInfoRegistration)
{
    nlohmann::json os;
    nlohmann::json networks;
    nlohmann::json ip;
    nlohmann::json address4;
    nlohmann::json address6;

    os["hostname"] = "test_name";
    os["os_name"] = "test_os";
    os["sysname"] = "test_type";
    os["os_version"] = "1.0.0";
    os["architecture"] = "test_arch";

    networks["iface"] = nlohmann::json::array();

    ip["state"] = "up";

    ip["IPv4"] = nlohmann::json::array();
    address4["address"] = "127.0.0.1";
    ip["IPv4"].push_back(address4);

    ip["IPv6"] = nlohmann::json::array();
    address6["address"] = "fe80::0000";
    ip["IPv6"].push_back(address6);

    networks["iface"].push_back(ip);

    const AgentInfo agentInfo(".", [os]() { return os; }, [networks]() { return networks; }, true);

    auto metadataInfo = nlohmann::json::parse(agentInfo.GetMetadataInfo());

    EXPECT_TRUE(metadataInfo != nullptr);

    // Agent information
    EXPECT_EQ(metadataInfo["type"], agentInfo.GetType());
    EXPECT_EQ(metadataInfo["version"], agentInfo.GetVersion());
    EXPECT_EQ(metadataInfo["id"], agentInfo.GetUUID());
    EXPECT_EQ(metadataInfo["name"], agentInfo.GetName());
    EXPECT_EQ(metadataInfo["key"], agentInfo.GetKey());
    EXPECT_TRUE(metadataInfo["groups"] == nullptr);

    // Endpoint information
    EXPECT_TRUE(metadataInfo["host"] != nullptr);
    EXPECT_TRUE(metadataInfo["host"]["os"] != nullptr);
    EXPECT_EQ(metadataInfo["host"]["os"]["name"], "test_os");
    EXPECT_EQ(metadataInfo["host"]["os"]["type"], "test_type");
    EXPECT_EQ(metadataInfo["host"]["os"]["version"], "1.0.0");
    EXPECT_TRUE(metadataInfo["host"]["ip"] != nullptr);
    EXPECT_EQ(metadataInfo["host"]["ip"][0], "127.0.0.1");
    EXPECT_EQ(metadataInfo["host"]["ip"][1], "fe80::0000");
    EXPECT_TRUE(metadataInfo["host"]["ip"][2] == nullptr);
    EXPECT_EQ(metadataInfo["host"]["architecture"], "test_arch");
    EXPECT_EQ(metadataInfo["host"]["hostname"], "test_name");
}

TEST_F(AgentInfoTest, TestLoadMetadataInfoConnected)
{
    nlohmann::json os;
    nlohmann::json networks;
    nlohmann::json ip;
    nlohmann::json address;

    os["hostname"] = "test_name";
    os["os_name"] = "test_os";
    os["sysname"] = "test_type";
    os["os_version"] = "1.0.0";
    os["architecture"] = "test_arch";

    networks["iface"] = nlohmann::json::array();

    ip["state"] = "up";
    ip["IPv4"] = nlohmann::json::array();

    address["address"] = "127.0.0.1";
    ip["IPv4"].push_back(address);

    networks["iface"].push_back(ip);

    const AgentInfo agentInfo(".", [os]() { return os; }, [networks]() { return networks; });

    auto metadataInfo = nlohmann::json::parse(agentInfo.GetMetadataInfo());

    EXPECT_TRUE(metadataInfo["agent"] != nullptr);

    // Agent information
    EXPECT_EQ(metadataInfo["agent"]["type"], agentInfo.GetType());
    EXPECT_EQ(metadataInfo["agent"]["version"], agentInfo.GetVersion());
    EXPECT_EQ(metadataInfo["agent"]["id"], agentInfo.GetUUID());
    EXPECT_EQ(metadataInfo["agent"]["name"], agentInfo.GetName());
    EXPECT_TRUE(metadataInfo["agent"]["key"] == nullptr);
    EXPECT_TRUE(metadataInfo["agent"]["groups"] != nullptr);

    // Endpoint information
    EXPECT_TRUE(metadataInfo["agent"]["host"] != nullptr);
    EXPECT_TRUE(metadataInfo["agent"]["host"]["os"] != nullptr);
    EXPECT_EQ(metadataInfo["agent"]["host"]["os"]["name"], "test_os");
    EXPECT_EQ(metadataInfo["agent"]["host"]["os"]["type"], "test_type");
    EXPECT_EQ(metadataInfo["agent"]["host"]["os"]["version"], "1.0.0");
    EXPECT_TRUE(metadataInfo["agent"]["host"]["ip"] != nullptr);
    EXPECT_EQ(metadataInfo["agent"]["host"]["ip"][0], "127.0.0.1");
    EXPECT_EQ(metadataInfo["agent"]["host"]["architecture"], "test_arch");
    EXPECT_EQ(metadataInfo["agent"]["host"]["hostname"], "test_name");
}

TEST_F(AgentInfoTest, TestLoadHeaderInfo)
{
    const AgentInfo agentInfo(".");

    auto headerInfo = agentInfo.GetHeaderInfo();

    EXPECT_NE(headerInfo, "");
    EXPECT_TRUE(headerInfo.starts_with("WazuhXDR/" + agentInfo.GetVersion() + " (" + agentInfo.GetType() + "; "));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
