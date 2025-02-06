#include <gtest/gtest.h>

#include <agent_info.hpp>
#include <agent_info_persistance.hpp>
#include <mocks_persistence.hpp>

#include <memory>
#include <string>
#include <vector>

class AgentInfoTest : public ::testing::Test
{
protected:
    MockPersistence* mockPersistence = nullptr;
    std::shared_ptr<AgentInfoPersistance> agentPersistence;
    std::unique_ptr<AgentInfo> agentInfo;

    void SetUp() override
    {
        InitializeAgentInfo();
    }

    void InitializeAgentInfo(const std::function<nlohmann::json()>& osLambda = nullptr,
                             const std::function<nlohmann::json()>& networksLambda = nullptr,
                             bool agentIsRegistering = false)
    {
        auto mockPersistencePtr = std::make_unique<MockPersistence>();
        mockPersistence = mockPersistencePtr.get();

        SetUpPersistenceMock();

        if (!agentIsRegistering)
        {
            SetUpAgentInfoInitialization();
        }

        agentPersistence = std::make_shared<AgentInfoPersistance>("db_path", std::move(mockPersistencePtr));

        agentInfo = std::make_unique<AgentInfo>("db_path",
                                                osLambda ? osLambda : nullptr,
                                                networksLambda ? networksLambda : nullptr,
                                                agentIsRegistering,
                                                std::move(agentPersistence));
    }

    void SetUpPersistenceMock()
    {
        EXPECT_CALL(*mockPersistence, TableExists("agent_info")).WillOnce(testing::Return(true));
        EXPECT_CALL(*mockPersistence, TableExists("agent_group")).WillOnce(testing::Return(true));
        EXPECT_CALL(*mockPersistence, GetCount("agent_info", testing::_, testing::_)).WillOnce(testing::Return(1));
    }

    void SetUpAgentInfoInitialization()
    {
        std::vector<column::Row> mockRowName = {{column::ColumnValue("name", column::ColumnType::TEXT, "name_test")}};
        std::vector<column::Row> mockRowKey = {{column::ColumnValue("key", column::ColumnType::TEXT, "key_test")}};
        std::vector<column::Row> mockRowUUID = {{column::ColumnValue("uuid", column::ColumnType::TEXT, "uuid_test")}};
        std::vector<column::Row> mockRowGroup = {{column::ColumnValue("name", column::ColumnType::TEXT, "group_test")}};

        testing::Sequence seq;
        EXPECT_CALL(*mockPersistence,
                    Select("agent_info", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
            .InSequence(seq)
            .WillOnce(testing::Return(mockRowName))
            .WillOnce(testing::Return(mockRowKey))
            .WillOnce(testing::Return(mockRowUUID));
        EXPECT_CALL(*mockPersistence,
                    Select("agent_group", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
            .WillOnce(testing::Return(mockRowGroup));
    }
};

TEST_F(AgentInfoTest, TestDefaultConstructorDefaultValues)
{
    EXPECT_NO_THROW({
        EXPECT_EQ(agentInfo->GetName(), "name_test");
        EXPECT_EQ(agentInfo->GetKey(), "key_test");
        EXPECT_NE(agentInfo->GetUUID(), "");
    });
}

TEST_F(AgentInfoTest, TestSetName)
{
    const std::string newName = "new_name";

    agentInfo->SetName(newName);
    EXPECT_EQ(agentInfo->GetName(), newName);
}

TEST_F(AgentInfoTest, TestSetKey)
{
    const std::string newKey = "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrNj";

    agentInfo->SetKey(newKey);
    EXPECT_EQ(agentInfo->GetKey(), newKey);
}

TEST_F(AgentInfoTest, TestSetBadKey)
{
    const std::string newKey1 = "4GhT7uFm";
    const std::string newKey2 = "4GhT7uFm1zQa9c2Vb7Lk8pYsX0WqZrN=";

    ASSERT_FALSE(agentInfo->SetKey(newKey1));
    ASSERT_FALSE(agentInfo->SetKey(newKey2));
}

TEST_F(AgentInfoTest, TestSetEmptyKey)
{
    const std::string newKey;

    agentInfo->SetKey(newKey);
    EXPECT_NE(agentInfo->GetKey(), newKey);
}

TEST_F(AgentInfoTest, TestSetUUID)
{
    const std::string newUUID = "new_uuid";

    agentInfo->SetUUID(newUUID);
    EXPECT_EQ(agentInfo->GetUUID(), newUUID);
}

TEST_F(AgentInfoTest, TestSetGroups)
{
    const std::vector<std::string> newGroups = {"t_group_1", "t_group_2"};

    agentInfo->SetGroups(newGroups);
    EXPECT_EQ(agentInfo->GetGroups(), newGroups);
}

TEST_F(AgentInfoTest, TestSaveGroups)
{
    const std::vector<std::string> newGroups = {"t_group_1", "t_group_2"};

    EXPECT_CALL(*mockPersistence, BeginTransaction()).Times(1);
    EXPECT_CALL(*mockPersistence, Remove("agent_group", testing::_, testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, Insert("agent_group", testing::_)).Times(2);
    EXPECT_CALL(*mockPersistence, CommitTransaction(testing::_)).Times(1);

    agentInfo->SetGroups(newGroups);
    EXPECT_TRUE(agentInfo->SaveGroups());
    EXPECT_EQ(agentInfo->GetGroups(), newGroups);
}

TEST_F(AgentInfoTest, TestSave)
{
    // Mock for: m_persistence->ResetToDefault();
    EXPECT_CALL(*mockPersistence, DropTable("agent_info")).Times(1);
    EXPECT_CALL(*mockPersistence, DropTable("agent_group")).Times(1);
    EXPECT_CALL(*mockPersistence, CreateTable(testing::_, testing::_)).Times(2);
    EXPECT_CALL(*mockPersistence, GetCount("agent_info", testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_CALL(*mockPersistence, Insert(testing::_, testing::_)).Times(1);

    // Mock for: m_persistence->SetName(m_name); m_persistence->SetKey(m_key); m_persistence->SetUUID(m_uuid);
    EXPECT_CALL(*mockPersistence, Update("agent_info", testing::_, testing::_, testing::_)).Times(3);

    // Mock for: m_persistence->SetGroups(m_groups);
    EXPECT_CALL(*mockPersistence, BeginTransaction()).Times(1);
    EXPECT_CALL(*mockPersistence, Remove("agent_group", testing::_, testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, Insert("agent_group", testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, CommitTransaction(testing::_)).Times(1);
    agentInfo->Save();
}

TEST_F(AgentInfoTest, TestLoadMetadataInfoNoSysInfo)
{
    InitializeAgentInfo(nullptr, nullptr, true);

    auto metadataInfo = nlohmann::json::parse(agentInfo->GetMetadataInfo());

    EXPECT_TRUE(metadataInfo != nullptr);

    // Agent information
    EXPECT_EQ(metadataInfo["type"], agentInfo->GetType());
    EXPECT_EQ(metadataInfo["version"], agentInfo->GetVersion());
    EXPECT_EQ(metadataInfo["id"], agentInfo->GetUUID());
    EXPECT_EQ(metadataInfo["name"], agentInfo->GetName());
    EXPECT_EQ(metadataInfo["key"], agentInfo->GetKey());
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

    InitializeAgentInfo([os]() { return os; }, [networks]() { return networks; }, true);

    auto metadataInfo = nlohmann::json::parse(agentInfo->GetMetadataInfo());

    EXPECT_TRUE(metadataInfo != nullptr);

    // Agent information
    EXPECT_EQ(metadataInfo["type"], agentInfo->GetType());
    EXPECT_EQ(metadataInfo["version"], agentInfo->GetVersion());
    EXPECT_EQ(metadataInfo["id"], agentInfo->GetUUID());
    EXPECT_EQ(metadataInfo["name"], agentInfo->GetName());
    EXPECT_EQ(metadataInfo["key"], agentInfo->GetKey());
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

    InitializeAgentInfo([os]() { return os; }, [networks]() { return networks; });

    auto metadataInfo = nlohmann::json::parse(agentInfo->GetMetadataInfo());

    EXPECT_TRUE(metadataInfo["agent"] != nullptr);

    // Agent information
    EXPECT_EQ(metadataInfo["agent"]["type"], agentInfo->GetType());
    EXPECT_EQ(metadataInfo["agent"]["version"], agentInfo->GetVersion());
    EXPECT_EQ(metadataInfo["agent"]["id"], agentInfo->GetUUID());
    EXPECT_EQ(metadataInfo["agent"]["name"], agentInfo->GetName());
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
    auto headerInfo = agentInfo->GetHeaderInfo();

    EXPECT_NE(headerInfo, "");
    EXPECT_TRUE(headerInfo.starts_with("WazuhXDR/" + agentInfo->GetVersion() + " (" + agentInfo->GetType() + "; "));
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
