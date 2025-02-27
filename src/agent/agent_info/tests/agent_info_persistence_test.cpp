#include <gtest/gtest.h>

#include <agent_info_persistence.hpp>
#include <mocks_persistence.hpp>

#include <memory>
#include <string>
#include <vector>

class AgentInfoPersistenceTest : public ::testing::Test
{
protected:
    MockPersistence* mockPersistence = nullptr;
    std::unique_ptr<AgentInfoPersistence> agentInfoPersistence;

    void SetUp() override
    {
        auto mockPersistencePtr = std::make_unique<MockPersistence>();
        mockPersistence = mockPersistencePtr.get();

        EXPECT_CALL(*mockPersistence, TableExists("agent_info")).WillOnce(testing::Return(true));
        EXPECT_CALL(*mockPersistence, TableExists("agent_group")).WillOnce(testing::Return(true));
        EXPECT_CALL(*mockPersistence, GetCount("agent_info", testing::_, testing::_))
            .WillOnce(testing::Return(0))
            .WillOnce(testing::Return(0));
        EXPECT_CALL(*mockPersistence, Insert("agent_info", testing::_)).Times(1);

        agentInfoPersistence = std::make_unique<AgentInfoPersistence>("db_path", std::move(mockPersistencePtr));
    }
};

TEST_F(AgentInfoPersistenceTest, TestConstruction)
{
    EXPECT_NE(agentInfoPersistence, nullptr);
}

TEST_F(AgentInfoPersistenceTest, TestGetNameValue)
{
    const std::vector<column::Row> mockRowName = {{column::ColumnValue("name", column::ColumnType::TEXT, "name_test")}};
    EXPECT_CALL(*mockPersistence,
                Select("agent_info", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRowName));
    EXPECT_EQ(agentInfoPersistence->GetName(), "name_test");
}

TEST_F(AgentInfoPersistenceTest, TestGetNameNotValue)
{
    const std::vector<column::Row> mockRowName = {};
    EXPECT_CALL(*mockPersistence,
                Select("agent_info", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRowName));
    EXPECT_EQ(agentInfoPersistence->GetName(), "");
}

TEST_F(AgentInfoPersistenceTest, TestGetNameCatch)
{
    EXPECT_CALL(*mockPersistence,
                Select("agent_info", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Select")));
    EXPECT_EQ(agentInfoPersistence->GetName(), "");
}

TEST_F(AgentInfoPersistenceTest, TestGetKeyValue)
{
    const std::vector<column::Row> mockRowKey = {{column::ColumnValue("key", column::ColumnType::TEXT, "key_test")}};
    EXPECT_CALL(*mockPersistence,
                Select("agent_info", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRowKey));
    EXPECT_EQ(agentInfoPersistence->GetKey(), "key_test");
}

TEST_F(AgentInfoPersistenceTest, TestGetKeyNotValue)
{
    const std::vector<column::Row> mockRowKey = {};
    EXPECT_CALL(*mockPersistence,
                Select("agent_info", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRowKey));
    EXPECT_EQ(agentInfoPersistence->GetKey(), "");
}

TEST_F(AgentInfoPersistenceTest, TestGetKeyCatch)
{
    EXPECT_CALL(*mockPersistence,
                Select("agent_info", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Select")));
    EXPECT_EQ(agentInfoPersistence->GetKey(), "");
}

TEST_F(AgentInfoPersistenceTest, TestGetUUIDValue)
{
    const std::vector<column::Row> mockRowUUID = {{column::ColumnValue("uuid", column::ColumnType::TEXT, "uuid_test")}};
    EXPECT_CALL(*mockPersistence,
                Select("agent_info", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRowUUID));
    EXPECT_EQ(agentInfoPersistence->GetUUID(), "uuid_test");
}

TEST_F(AgentInfoPersistenceTest, TestGetUUIDNotValue)
{
    const std::vector<column::Row> mockRowUUID = {};
    EXPECT_CALL(*mockPersistence,
                Select("agent_info", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRowUUID));
    EXPECT_EQ(agentInfoPersistence->GetUUID(), "");
}

TEST_F(AgentInfoPersistenceTest, TestGetUUIDCatch)
{
    EXPECT_CALL(*mockPersistence,
                Select("agent_info", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Select")));
    EXPECT_EQ(agentInfoPersistence->GetUUID(), "");
}

TEST_F(AgentInfoPersistenceTest, TestGetGroupsValue)
{
    const std::vector<column::Row> mockRowGroups = {{column::ColumnValue("name", column::ColumnType::TEXT, "group_1")},
                                                    {column::ColumnValue("name", column::ColumnType::TEXT, "group_2")}};
    EXPECT_CALL(*mockPersistence,
                Select("agent_group", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRowGroups));

    const std::vector<std::string> expectedGroups = {"group_1", "group_2"};
    EXPECT_EQ(agentInfoPersistence->GetGroups(), expectedGroups);
}

TEST_F(AgentInfoPersistenceTest, TestGetGroupsNotValue)
{
    const std::vector<column::Row> mockRowGroups = {};
    EXPECT_CALL(*mockPersistence,
                Select("agent_group", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRowGroups));

    const std::vector<std::string> expectedGroups = {};
    EXPECT_EQ(agentInfoPersistence->GetGroups(), expectedGroups);
}

TEST_F(AgentInfoPersistenceTest, TestGetGroupsCatch)
{
    EXPECT_CALL(*mockPersistence,
                Select("agent_group", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Select")));

    const std::vector<std::string> expectedGroups = {};
    EXPECT_EQ(agentInfoPersistence->GetGroups(), expectedGroups);
}

TEST_F(AgentInfoPersistenceTest, TestSetName)
{
    const std::string expectedColumn = "name";
    const std::string newName = "new_name";

    EXPECT_CALL(*mockPersistence,
                Update(testing::Eq("agent_info"),
                       testing::AllOf(testing::SizeIs(1),
                                      testing::Contains(
                                          testing::AllOf(testing::Field(&column::ColumnValue::Value, newName),
                                                         testing::Field(&column::ColumnName::Name, expectedColumn)))),
                       testing::_,
                       testing::_))
        .Times(1);
    EXPECT_TRUE(agentInfoPersistence->SetName(newName));
}

TEST_F(AgentInfoPersistenceTest, TestSetNameCatch)
{
    const std::string expectedColumn = "name";
    const std::string newName = "new_name";

    EXPECT_CALL(*mockPersistence,
                Update(testing::Eq("agent_info"),
                       testing::AllOf(testing::SizeIs(1),
                                      testing::Contains(
                                          testing::AllOf(testing::Field(&column::ColumnValue::Value, newName),
                                                         testing::Field(&column::ColumnName::Name, expectedColumn)))),
                       testing::_,
                       testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Update")));
    EXPECT_FALSE(agentInfoPersistence->SetName(newName));
}

TEST_F(AgentInfoPersistenceTest, TestSetKey)
{
    const std::string expectedColumn = "key";
    const std::string newKey = "new_key";

    EXPECT_CALL(*mockPersistence,
                Update(testing::Eq("agent_info"),
                       testing::AllOf(testing::SizeIs(1),
                                      testing::Contains(
                                          testing::AllOf(testing::Field(&column::ColumnValue::Value, newKey),
                                                         testing::Field(&column::ColumnName::Name, expectedColumn)))),
                       testing::_,
                       testing::_))
        .Times(1);
    EXPECT_TRUE(agentInfoPersistence->SetKey(newKey));
}

TEST_F(AgentInfoPersistenceTest, TestSetKeyCatch)
{
    const std::string expectedColumn = "key";
    const std::string newKey = "new_key";

    EXPECT_CALL(*mockPersistence,
                Update(testing::Eq("agent_info"),
                       testing::AllOf(testing::SizeIs(1),
                                      testing::Contains(
                                          testing::AllOf(testing::Field(&column::ColumnValue::Value, newKey),
                                                         testing::Field(&column::ColumnName::Name, expectedColumn)))),
                       testing::_,
                       testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Update")));
    EXPECT_FALSE(agentInfoPersistence->SetKey(newKey));
}

TEST_F(AgentInfoPersistenceTest, TestSetUUID)
{
    const std::string expectedColumn = "uuid";
    const std::string newUUID = "new_uuid";

    EXPECT_CALL(*mockPersistence,
                Update(testing::Eq("agent_info"),
                       testing::AllOf(testing::SizeIs(1),
                                      testing::Contains(
                                          testing::AllOf(testing::Field(&column::ColumnValue::Value, newUUID),
                                                         testing::Field(&column::ColumnName::Name, expectedColumn)))),
                       testing::_,
                       testing::_))
        .Times(1);
    EXPECT_TRUE(agentInfoPersistence->SetUUID(newUUID));
}

TEST_F(AgentInfoPersistenceTest, TestSetUUIDCatch)
{
    const std::string expectedColumn = "uuid";
    const std::string newUUID = "new_uuid";

    EXPECT_CALL(*mockPersistence,
                Update(testing::Eq("agent_info"),
                       testing::AllOf(testing::SizeIs(1),
                                      testing::Contains(
                                          testing::AllOf(testing::Field(&column::ColumnValue::Value, newUUID),
                                                         testing::Field(&column::ColumnName::Name, expectedColumn)))),
                       testing::_,
                       testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Update")));
    EXPECT_FALSE(agentInfoPersistence->SetUUID(newUUID));
}

TEST_F(AgentInfoPersistenceTest, TestSetGroupsSuccess)
{
    const std::vector<std::string> newGroups = {"t_group_1", "t_group_2"};

    EXPECT_CALL(*mockPersistence, BeginTransaction()).Times(1);
    EXPECT_CALL(*mockPersistence, Remove("agent_group", testing::_, testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, Insert("agent_group", testing::_)).Times(2);
    EXPECT_CALL(*mockPersistence, CommitTransaction(testing::_)).Times(1);

    EXPECT_TRUE(agentInfoPersistence->SetGroups(newGroups));
}

TEST_F(AgentInfoPersistenceTest, TestSetGroupsBeginTransactionFails)
{
    const std::vector<std::string> newGroups = {"t_group_1", "t_group_2"};

    EXPECT_CALL(*mockPersistence, BeginTransaction())
        .WillOnce(testing::Throw(std::runtime_error("Error BeginTransaction")));

    EXPECT_FALSE(agentInfoPersistence->SetGroups(newGroups));
}

TEST_F(AgentInfoPersistenceTest, TestSetGroupsRemoveFails)
{
    const std::vector<std::string> newGroups = {"t_group_1", "t_group_2"};

    EXPECT_CALL(*mockPersistence, BeginTransaction()).Times(1);
    EXPECT_CALL(*mockPersistence, Remove("agent_group", testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Remove")));
    EXPECT_CALL(*mockPersistence, RollbackTransaction(testing::_)).Times(1);

    EXPECT_FALSE(agentInfoPersistence->SetGroups(newGroups));
}

TEST_F(AgentInfoPersistenceTest, TestSetGroupsInsertFails1)
{
    const std::vector<std::string> newGroups = {"t_group_1", "t_group_2"};

    EXPECT_CALL(*mockPersistence, BeginTransaction()).Times(1);
    EXPECT_CALL(*mockPersistence, Remove("agent_group", testing::_, testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, Insert("agent_group", testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Insert")));
    EXPECT_CALL(*mockPersistence, RollbackTransaction(testing::_)).Times(1);

    EXPECT_FALSE(agentInfoPersistence->SetGroups(newGroups));
}

TEST_F(AgentInfoPersistenceTest, TestSetGroupsInsertFails2)
{
    const std::vector<std::string> newGroups = {"t_group_1", "t_group_2"};

    EXPECT_CALL(*mockPersistence, BeginTransaction()).Times(1);
    EXPECT_CALL(*mockPersistence, Remove("agent_group", testing::_, testing::_)).Times(1);

    const testing::Sequence seq;
    EXPECT_CALL(*mockPersistence, Insert("agent_group", testing::_))
        .InSequence(seq)
        .WillOnce(testing::Return())
        .WillOnce(testing::Throw(std::runtime_error("Error Insert")));
    EXPECT_CALL(*mockPersistence, RollbackTransaction(testing::_)).Times(1);

    EXPECT_FALSE(agentInfoPersistence->SetGroups(newGroups));
}

TEST_F(AgentInfoPersistenceTest, TestSetGroupsCommitFails)
{
    const std::vector<std::string> newGroups = {"t_group_1", "t_group_2"};

    EXPECT_CALL(*mockPersistence, BeginTransaction()).Times(1);
    EXPECT_CALL(*mockPersistence, Remove("agent_group", testing::_, testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, Insert("agent_group", testing::_)).Times(2);
    EXPECT_CALL(*mockPersistence, CommitTransaction(testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Commit")));
    EXPECT_CALL(*mockPersistence, RollbackTransaction(testing::_)).Times(1);

    EXPECT_FALSE(agentInfoPersistence->SetGroups(newGroups));
}

TEST_F(AgentInfoPersistenceTest, TestSetGroupsRollbackFails)
{
    const std::vector<std::string> newGroups = {"t_group_1", "t_group_2"};

    EXPECT_CALL(*mockPersistence, BeginTransaction()).Times(1);
    EXPECT_CALL(*mockPersistence, Remove("agent_group", testing::_, testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, Insert("agent_group", testing::_)).Times(2);
    EXPECT_CALL(*mockPersistence, CommitTransaction(testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Commit")));
    EXPECT_CALL(*mockPersistence, RollbackTransaction(testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Rollback")));

    EXPECT_FALSE(agentInfoPersistence->SetGroups(newGroups));
}

TEST_F(AgentInfoPersistenceTest, TestResetToDefaultSuccess)
{
    EXPECT_CALL(*mockPersistence, DropTable("agent_info")).Times(1);
    EXPECT_CALL(*mockPersistence, DropTable("agent_group")).Times(1);
    EXPECT_CALL(*mockPersistence, CreateTable("agent_info", testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, CreateTable("agent_group", testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, GetCount("agent_info", testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_CALL(*mockPersistence, Insert(testing::_, testing::_)).Times(1);

    EXPECT_TRUE(agentInfoPersistence->ResetToDefault());
}

TEST_F(AgentInfoPersistenceTest, TestResetToDefaultDropTableAgentInfoFails)
{
    EXPECT_CALL(*mockPersistence, DropTable("agent_info"))
        .WillOnce(testing::Throw(std::runtime_error("Error DropTable")));

    EXPECT_FALSE(agentInfoPersistence->ResetToDefault());
}

TEST_F(AgentInfoPersistenceTest, TestResetToDefaultDropTableAgentGroupFails)
{
    EXPECT_CALL(*mockPersistence, DropTable("agent_info")).Times(1);
    EXPECT_CALL(*mockPersistence, DropTable("agent_group"))
        .WillOnce(testing::Throw(std::runtime_error("Error DropTable")));

    EXPECT_FALSE(agentInfoPersistence->ResetToDefault());
}

TEST_F(AgentInfoPersistenceTest, TestResetToDefaultCreateAgentInfoTableFails)
{
    EXPECT_CALL(*mockPersistence, DropTable("agent_info")).Times(1);
    EXPECT_CALL(*mockPersistence, DropTable("agent_group")).Times(1);
    EXPECT_CALL(*mockPersistence, CreateTable("agent_info", testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error CreateAgentInfoTable")));

    EXPECT_FALSE(agentInfoPersistence->ResetToDefault());
}

TEST_F(AgentInfoPersistenceTest, TestResetToDefaultCreateAgentGroupTableFails)
{
    EXPECT_CALL(*mockPersistence, DropTable("agent_info")).Times(1);
    EXPECT_CALL(*mockPersistence, DropTable("agent_group")).Times(1);
    EXPECT_CALL(*mockPersistence, CreateTable("agent_info", testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, CreateTable("agent_group", testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error CreateAgentGroupTable")));

    EXPECT_FALSE(agentInfoPersistence->ResetToDefault());
}

TEST_F(AgentInfoPersistenceTest, TestResetToDefaultInsertFails)
{
    EXPECT_CALL(*mockPersistence, DropTable("agent_info")).Times(1);
    EXPECT_CALL(*mockPersistence, DropTable("agent_group")).Times(1);
    EXPECT_CALL(*mockPersistence, CreateTable("agent_info", testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, CreateTable("agent_group", testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, GetCount("agent_info", testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_CALL(*mockPersistence, Insert("agent_info", testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Insert")));

    EXPECT_FALSE(agentInfoPersistence->ResetToDefault());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
