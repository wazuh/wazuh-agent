#include <gtest/gtest.h>

#include <agent_info.hpp>
#include <mock_agent_info_persistence.hpp>

#include <memory>
#include <string>
#include <vector>

class AgentInfoTest : public ::testing::Test
{
protected:
    std::shared_ptr<MockAgentInfoPersistence> m_agentPersistence;
    std::unique_ptr<AgentInfo> m_agentInfo;

    void SetUp() override
    {
        InitializeAgentInfo();
    }

    void InitializeAgentInfo(const std::function<nlohmann::json()>& osLambda = nullptr,
                             const std::function<nlohmann::json()>& networksLambda = nullptr,
                             bool agentIsEnrolling = false)
    {
        m_agentPersistence = std::make_shared<MockAgentInfoPersistence>();

        if (!agentIsEnrolling)
        {
            SetUpAgentInfoInitialization();
        }

        m_agentInfo = std::make_unique<AgentInfo>("db_path",
                                                  osLambda ? osLambda : nullptr,
                                                  networksLambda ? networksLambda : nullptr,
                                                  agentIsEnrolling,
                                                  m_agentPersistence);
    }

    void SetUpAgentInfoInitialization()
    {
        const std::string mockName = "name_test";
        const std::string mockKey = "key_test";
        const std::string mockUUID = "uuid_test";
        const std::vector<std::string> mockGroup = {};

        EXPECT_CALL(*m_agentPersistence, GetName()).WillOnce(testing::Return(mockName));
        EXPECT_CALL(*m_agentPersistence, GetKey()).WillOnce(testing::Return(mockKey));
        EXPECT_CALL(*m_agentPersistence, GetUUID()).WillOnce(testing::Return(mockUUID));
        EXPECT_CALL(*m_agentPersistence, GetGroups()).WillOnce(testing::Return(mockGroup));
    }
};

TEST_F(AgentInfoTest, TestDefaultConstructorDefaultValues)
{
    EXPECT_NO_THROW({
        EXPECT_EQ(m_agentInfo->GetName(), "name_test");
        EXPECT_EQ(m_agentInfo->GetKey(), "key_test");
        EXPECT_NE(m_agentInfo->GetUUID(), "");
    });
}

TEST_F(AgentInfoTest, TestSetName)
{
    const std::string newName = "new_name";

    m_agentInfo->SetName(newName);
    EXPECT_EQ(m_agentInfo->GetName(), newName);
}

TEST_F(AgentInfoTest, TestSetKey)
{
    const std::string newKey = "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj";

    m_agentInfo->SetKey(newKey);
    EXPECT_EQ(m_agentInfo->GetKey(), newKey);
}

TEST_F(AgentInfoTest, TestSetBadKey)
{
    const std::string newKey1 = "4GhT7uFm";
    const std::string newKey2 = "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrN=";

    ASSERT_FALSE(m_agentInfo->SetKey(newKey1));
    ASSERT_FALSE(m_agentInfo->SetKey(newKey2));
}

TEST_F(AgentInfoTest, TestSetEmptyKey)
{
    const std::string newKey;

    m_agentInfo->SetKey(newKey);
    EXPECT_NE(m_agentInfo->GetKey(), newKey);
}

TEST_F(AgentInfoTest, TestSetUUID)
{
    const std::string newUUID = "new_uuid";

    m_agentInfo->SetUUID(newUUID);
    EXPECT_EQ(m_agentInfo->GetUUID(), newUUID);
}

TEST_F(AgentInfoTest, TestSetGroups)
{
    const std::vector<std::string> newGroups = {"t_group_1", "t_group_2"};

    m_agentInfo->SetGroups(newGroups);
    EXPECT_EQ(m_agentInfo->GetGroups(), newGroups);
}

TEST_F(AgentInfoTest, TestSaveGroupsFail)
{
    EXPECT_CALL(*m_agentPersistence, SetGroups(testing::_)).WillOnce(testing::Return(false));

    EXPECT_FALSE(m_agentInfo->SaveGroups());
}

TEST_F(AgentInfoTest, TestSaveGroups)
{
    EXPECT_CALL(*m_agentPersistence, SetGroups(testing::_)).WillOnce(testing::Return(true));

    EXPECT_TRUE(m_agentInfo->SaveGroups());
}

TEST_F(AgentInfoTest, TestSave)
{
    EXPECT_CALL(*m_agentPersistence, ResetToDefault()).Times(1);
    EXPECT_CALL(*m_agentPersistence, SetName(testing::_)).Times(1);
    EXPECT_CALL(*m_agentPersistence, SetKey(testing::_)).Times(1);
    EXPECT_CALL(*m_agentPersistence, SetUUID(testing::_)).Times(1);
    EXPECT_CALL(*m_agentPersistence, SetGroups(testing::_)).Times(1);

    m_agentInfo->Save();
}

TEST_F(AgentInfoTest, TestLoadMetadataInfoNoSysInfo)
{
    InitializeAgentInfo(nullptr, nullptr, true);

    const auto metadataInfo = nlohmann::json::parse(m_agentInfo->GetMetadataInfo());

    EXPECT_TRUE(metadataInfo != nullptr);

    // Agent information
    EXPECT_EQ(metadataInfo["type"], m_agentInfo->GetType());
    EXPECT_EQ(metadataInfo["version"], m_agentInfo->GetVersion());
    EXPECT_EQ(metadataInfo["id"], m_agentInfo->GetUUID());
    EXPECT_EQ(metadataInfo["name"], m_agentInfo->GetName());
    EXPECT_EQ(metadataInfo["key"], m_agentInfo->GetKey());
    EXPECT_FALSE(metadataInfo.contains("groups"));

    // Endpoint information
    EXPECT_FALSE(metadataInfo.contains("host"));
}

TEST_F(AgentInfoTest, TestLoadMetadataInfoEnrollment)
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

    InitializeAgentInfo([os]() { return os; }, [networks]() { return networks; }, true);

    const auto metadataInfo = nlohmann::json::parse(m_agentInfo->GetMetadataInfo());

    EXPECT_TRUE(metadataInfo != nullptr);

    // Agent information
    EXPECT_EQ(metadataInfo["type"], m_agentInfo->GetType());
    EXPECT_EQ(metadataInfo["version"], m_agentInfo->GetVersion());
    EXPECT_EQ(metadataInfo["id"], m_agentInfo->GetUUID());
    EXPECT_EQ(metadataInfo["name"], m_agentInfo->GetName());
    EXPECT_EQ(metadataInfo["key"], m_agentInfo->GetKey());
    EXPECT_FALSE(metadataInfo.contains("groups"));

    // Endpoint information
    EXPECT_TRUE(metadataInfo.contains("host"));
    EXPECT_TRUE(metadataInfo["host"].contains("os"));
    EXPECT_EQ(metadataInfo["host"]["os"]["name"], "test_os");
    EXPECT_EQ(metadataInfo["host"]["os"]["type"], "test_type");
    EXPECT_EQ(metadataInfo["host"]["os"]["version"], "1.0.0");
    EXPECT_TRUE(metadataInfo["host"].contains("ip"));
    EXPECT_EQ(metadataInfo["host"]["ip"][0], "127.0.0.1");
    EXPECT_EQ(metadataInfo["host"]["ip"][1], "fe80::0000");
    EXPECT_LT(metadataInfo["host"]["ip"].size(), 3);
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

    InitializeAgentInfo([os]() { return os; }, [networks]() { return networks; });

    const auto metadataInfo = nlohmann::json::parse(m_agentInfo->GetMetadataInfo());

    EXPECT_TRUE(metadataInfo.contains("agent"));

    // Agent information
    EXPECT_EQ(metadataInfo["agent"]["type"], m_agentInfo->GetType());
    EXPECT_EQ(metadataInfo["agent"]["version"], m_agentInfo->GetVersion());
    EXPECT_EQ(metadataInfo["agent"]["id"], m_agentInfo->GetUUID());
    EXPECT_EQ(metadataInfo["agent"]["name"], m_agentInfo->GetName());
    EXPECT_FALSE(metadataInfo["agent"].contains("key"));
    EXPECT_TRUE(metadataInfo["agent"].contains("groups"));

    // Endpoint information
    EXPECT_TRUE(metadataInfo["agent"].contains("host"));
    EXPECT_TRUE(metadataInfo["agent"]["host"].contains("os"));
    EXPECT_EQ(metadataInfo["agent"]["host"]["os"]["name"], "test_os");
    EXPECT_EQ(metadataInfo["agent"]["host"]["os"]["type"], "test_type");
    EXPECT_EQ(metadataInfo["agent"]["host"]["os"]["version"], "1.0.0");
    EXPECT_TRUE(metadataInfo["agent"]["host"].contains("ip"));
    EXPECT_EQ(metadataInfo["agent"]["host"]["ip"][0], "127.0.0.1");
    EXPECT_EQ(metadataInfo["agent"]["host"]["architecture"], "test_arch");
    EXPECT_EQ(metadataInfo["agent"]["host"]["hostname"], "test_name");
}

TEST_F(AgentInfoTest, TestLoadHeaderInfo)
{
    auto headerInfo = m_agentInfo->GetHeaderInfo();

    EXPECT_NE(headerInfo, "");
    EXPECT_TRUE(headerInfo.starts_with("WazuhXDR/" + m_agentInfo->GetVersion() + " (" + m_agentInfo->GetType() + "; "));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
