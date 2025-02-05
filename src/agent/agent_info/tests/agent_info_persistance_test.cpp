#include <gtest/gtest.h>

#include <agent_info_persistance.hpp>
#include <mocks_persistence.hpp>

#include <memory>
#include <string>
#include <vector>

class AgentInfoPersistanceTest : public ::testing::Test
{
protected:
    MockPersistence* mockPersistence = nullptr;
    std::unique_ptr<AgentInfoPersistance> agentInfoPersistance;

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

        agentInfoPersistance = std::make_unique<AgentInfoPersistance>("db_path", std::move(mockPersistencePtr));
    }
};

TEST_F(AgentInfoPersistanceTest, TestConstruction)
{
    EXPECT_NE(agentInfoPersistance, nullptr);
}

TEST_F(AgentInfoPersistanceTest, TestGetNameValue)
{
    std::vector<column::Row> mockRowName = {{column::ColumnValue("name", column::ColumnType::TEXT, "name_test")}};
    EXPECT_CALL(*mockPersistence,
                Select("agent_info", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRowName));
    EXPECT_EQ(agentInfoPersistance->GetName(), "name_test");
}

TEST_F(AgentInfoPersistanceTest, TestGetNameNotValue)
{
    std::vector<column::Row> mockRowName = {};
    EXPECT_CALL(*mockPersistence,
                Select("agent_info", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRowName));
    EXPECT_EQ(agentInfoPersistance->GetName(), "");
}

TEST_F(AgentInfoPersistanceTest, TestGetNameCatch)
{
    EXPECT_CALL(*mockPersistence,
                Select("agent_info", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Select")));
    EXPECT_EQ(agentInfoPersistance->GetName(), "");
}

TEST_F(AgentInfoPersistanceTest, TestGetKeyValue)
{
    std::vector<column::Row> mockRowKey = {{column::ColumnValue("key", column::ColumnType::TEXT, "key_test")}};
    EXPECT_CALL(*mockPersistence,
                Select("agent_info", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRowKey));
    EXPECT_EQ(agentInfoPersistance->GetKey(), "key_test");
}

TEST_F(AgentInfoPersistanceTest, TestGetKeyNotValue)
{
    std::vector<column::Row> mockRowKey = {};
    EXPECT_CALL(*mockPersistence,
                Select("agent_info", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRowKey));
    EXPECT_EQ(agentInfoPersistance->GetKey(), "");
}

TEST_F(AgentInfoPersistanceTest, TestGetKeyCatch)
{
    EXPECT_CALL(*mockPersistence,
                Select("agent_info", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Select")));
    EXPECT_EQ(agentInfoPersistance->GetKey(), "");
}

TEST_F(AgentInfoPersistanceTest, TestGetUUIDValue)
{
    std::vector<column::Row> mockRowUUID = {{column::ColumnValue("uuid", column::ColumnType::TEXT, "uuid_test")}};
    EXPECT_CALL(*mockPersistence,
                Select("agent_info", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRowUUID));
    EXPECT_EQ(agentInfoPersistance->GetUUID(), "uuid_test");
}

TEST_F(AgentInfoPersistanceTest, TestGetUUIDNotValue)
{
    std::vector<column::Row> mockRowUUID = {};
    EXPECT_CALL(*mockPersistence,
                Select("agent_info", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRowUUID));
    EXPECT_EQ(agentInfoPersistance->GetUUID(), "");
}

TEST_F(AgentInfoPersistanceTest, TestGetUUIDCatch)
{
    EXPECT_CALL(*mockPersistence,
                Select("agent_info", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Select")));
    EXPECT_EQ(agentInfoPersistance->GetUUID(), "");
}

TEST_F(AgentInfoPersistanceTest, TestGetGroupsValue)
{
    std::vector<column::Row> mockRowGroups = {{column::ColumnValue("name", column::ColumnType::TEXT, "group_1")},
                                              {column::ColumnValue("name", column::ColumnType::TEXT, "group_2")}};
    EXPECT_CALL(*mockPersistence,
                Select("agent_group", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRowGroups));

    std::vector<std::string> expectedGroups = {"group_1", "group_2"};
    EXPECT_EQ(agentInfoPersistance->GetGroups(), expectedGroups);
}

TEST_F(AgentInfoPersistanceTest, TestGetGroupsNotValue)
{
    std::vector<column::Row> mockRowGroups = {};
    EXPECT_CALL(*mockPersistence,
                Select("agent_group", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Return(mockRowGroups));

    std::vector<std::string> expectedGroups = {};
    EXPECT_EQ(agentInfoPersistance->GetGroups(), expectedGroups);
}

TEST_F(AgentInfoPersistanceTest, TestGetGroupsCatch)
{
    EXPECT_CALL(*mockPersistence,
                Select("agent_group", testing::_, testing::_, testing::_, testing::_, testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Select")));

    std::vector<std::string> expectedGroups = {};
    EXPECT_EQ(agentInfoPersistance->GetGroups(), expectedGroups);
}

TEST_F(AgentInfoPersistanceTest, TestSetName)
{
    std::string expectedColumn = "name";
    std::string newName = "new_name";

    EXPECT_CALL(*mockPersistence,
                Update(testing::Eq("agent_info"),
                       testing::AllOf(testing::SizeIs(1),
                                      testing::Contains(
                                          testing::AllOf(testing::Field(&column::ColumnValue::Value, newName),
                                                         testing::Field(&column::ColumnName::Name, expectedColumn)))),
                       testing::_,
                       testing::_))
        .Times(1);
    EXPECT_TRUE(agentInfoPersistance->SetName(newName));
}

TEST_F(AgentInfoPersistanceTest, TestSetNameCatch)
{
    std::string expectedColumn = "name";
    std::string newName = "new_name";

    EXPECT_CALL(*mockPersistence,
                Update(testing::Eq("agent_info"),
                       testing::AllOf(testing::SizeIs(1),
                                      testing::Contains(
                                          testing::AllOf(testing::Field(&column::ColumnValue::Value, newName),
                                                         testing::Field(&column::ColumnName::Name, expectedColumn)))),
                       testing::_,
                       testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Update")));
    EXPECT_FALSE(agentInfoPersistance->SetName(newName));
}

TEST_F(AgentInfoPersistanceTest, TestSetKey)
{
    std::string expectedColumn = "key";
    std::string newKey = "new_key";

    EXPECT_CALL(*mockPersistence,
                Update(testing::Eq("agent_info"),
                       testing::AllOf(testing::SizeIs(1),
                                      testing::Contains(
                                          testing::AllOf(testing::Field(&column::ColumnValue::Value, newKey),
                                                         testing::Field(&column::ColumnName::Name, expectedColumn)))),
                       testing::_,
                       testing::_))
        .Times(1);
    EXPECT_TRUE(agentInfoPersistance->SetKey(newKey));
}

TEST_F(AgentInfoPersistanceTest, TestSetKeyCatch)
{
    std::string expectedColumn = "key";
    std::string newKey = "new_key";

    EXPECT_CALL(*mockPersistence,
                Update(testing::Eq("agent_info"),
                       testing::AllOf(testing::SizeIs(1),
                                      testing::Contains(
                                          testing::AllOf(testing::Field(&column::ColumnValue::Value, newKey),
                                                         testing::Field(&column::ColumnName::Name, expectedColumn)))),
                       testing::_,
                       testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Update")));
    EXPECT_FALSE(agentInfoPersistance->SetKey(newKey));
}

TEST_F(AgentInfoPersistanceTest, TestSetUUID)
{
    std::string expectedColumn = "uuid";
    std::string newUUID = "new_uuid";

    EXPECT_CALL(*mockPersistence,
                Update(testing::Eq("agent_info"),
                       testing::AllOf(testing::SizeIs(1),
                                      testing::Contains(
                                          testing::AllOf(testing::Field(&column::ColumnValue::Value, newUUID),
                                                         testing::Field(&column::ColumnName::Name, expectedColumn)))),
                       testing::_,
                       testing::_))
        .Times(1);
    EXPECT_TRUE(agentInfoPersistance->SetUUID(newUUID));
}

TEST_F(AgentInfoPersistanceTest, TestSetUUIDCatch)
{
    std::string expectedColumn = "uuid";
    std::string newUUID = "new_uuid";

    EXPECT_CALL(*mockPersistence,
                Update(testing::Eq("agent_info"),
                       testing::AllOf(testing::SizeIs(1),
                                      testing::Contains(
                                          testing::AllOf(testing::Field(&column::ColumnValue::Value, newUUID),
                                                         testing::Field(&column::ColumnName::Name, expectedColumn)))),
                       testing::_,
                       testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Update")));
    EXPECT_FALSE(agentInfoPersistance->SetUUID(newUUID));
}

TEST_F(AgentInfoPersistanceTest, TestSetGroupsSuccess)
{
    const std::vector<std::string> newGroups = {"t_group_1", "t_group_2"};

    EXPECT_CALL(*mockPersistence, BeginTransaction()).Times(1);
    EXPECT_CALL(*mockPersistence, Remove("agent_group", testing::_, testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, Insert("agent_group", testing::_)).Times(2);
    EXPECT_CALL(*mockPersistence, CommitTransaction(testing::_)).Times(1);

    EXPECT_TRUE(agentInfoPersistance->SetGroups(newGroups));
}

TEST_F(AgentInfoPersistanceTest, TestSetGroupsBeginTransactionFails)
{
    const std::vector<std::string> newGroups = {"t_group_1", "t_group_2"};

    EXPECT_CALL(*mockPersistence, BeginTransaction())
        .WillOnce(testing::Throw(std::runtime_error("Error BeginTransaction")));

    EXPECT_FALSE(agentInfoPersistance->SetGroups(newGroups));
}

TEST_F(AgentInfoPersistanceTest, TestSetGroupsRemoveFails)
{
    const std::vector<std::string> newGroups = {"t_group_1", "t_group_2"};

    EXPECT_CALL(*mockPersistence, BeginTransaction()).Times(1);
    EXPECT_CALL(*mockPersistence, Remove("agent_group", testing::_, testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Remove")));
    EXPECT_CALL(*mockPersistence, RollbackTransaction(testing::_)).Times(1);

    EXPECT_FALSE(agentInfoPersistance->SetGroups(newGroups));
}

TEST_F(AgentInfoPersistanceTest, TestSetGroupsInsertFails1)
{
    const std::vector<std::string> newGroups = {"t_group_1", "t_group_2"};

    EXPECT_CALL(*mockPersistence, BeginTransaction()).Times(1);
    EXPECT_CALL(*mockPersistence, Remove("agent_group", testing::_, testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, Insert("agent_group", testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Insert")));
    EXPECT_CALL(*mockPersistence, RollbackTransaction(testing::_)).Times(1);

    EXPECT_FALSE(agentInfoPersistance->SetGroups(newGroups));
}

TEST_F(AgentInfoPersistanceTest, TestSetGroupsInsertFails2)
{
    const std::vector<std::string> newGroups = {"t_group_1", "t_group_2"};

    EXPECT_CALL(*mockPersistence, BeginTransaction()).Times(1);
    EXPECT_CALL(*mockPersistence, Remove("agent_group", testing::_, testing::_)).Times(1);

    testing::Sequence seq;
    EXPECT_CALL(*mockPersistence, Insert("agent_group", testing::_))
        .InSequence(seq)
        .WillOnce(testing::Return())
        .WillOnce(testing::Throw(std::runtime_error("Error Insert")));
    EXPECT_CALL(*mockPersistence, RollbackTransaction(testing::_)).Times(1);

    EXPECT_FALSE(agentInfoPersistance->SetGroups(newGroups));
}

TEST_F(AgentInfoPersistanceTest, TestSetGroupsCommitFails)
{
    const std::vector<std::string> newGroups = {"t_group_1", "t_group_2"};

    EXPECT_CALL(*mockPersistence, BeginTransaction()).Times(1);
    EXPECT_CALL(*mockPersistence, Remove("agent_group", testing::_, testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, Insert("agent_group", testing::_)).Times(2);
    EXPECT_CALL(*mockPersistence, CommitTransaction(testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Commit")));
    EXPECT_CALL(*mockPersistence, RollbackTransaction(testing::_)).Times(1);

    EXPECT_FALSE(agentInfoPersistance->SetGroups(newGroups));
}

TEST_F(AgentInfoPersistanceTest, TestSetGroupsRollbackFails)
{
    const std::vector<std::string> newGroups = {"t_group_1", "t_group_2"};

    EXPECT_CALL(*mockPersistence, BeginTransaction()).Times(1);
    EXPECT_CALL(*mockPersistence, Remove("agent_group", testing::_, testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, Insert("agent_group", testing::_)).Times(2);
    EXPECT_CALL(*mockPersistence, CommitTransaction(testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Commit")));
    EXPECT_CALL(*mockPersistence, RollbackTransaction(testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Rollback")));

    EXPECT_FALSE(agentInfoPersistance->SetGroups(newGroups));
}

TEST_F(AgentInfoPersistanceTest, TestResetToDefaultSuccess)
{
    EXPECT_CALL(*mockPersistence, DropTable("agent_info")).Times(1);
    EXPECT_CALL(*mockPersistence, DropTable("agent_group")).Times(1);
    EXPECT_CALL(*mockPersistence, CreateTable("agent_info", testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, CreateTable("agent_group", testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, GetCount("agent_info", testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_CALL(*mockPersistence, Insert(testing::_, testing::_)).Times(1);

    EXPECT_TRUE(agentInfoPersistance->ResetToDefault());
}

TEST_F(AgentInfoPersistanceTest, TestResetToDefaultDropTableAgentInfoFails)
{
    EXPECT_CALL(*mockPersistence, DropTable("agent_info"))
        .WillOnce(testing::Throw(std::runtime_error("Error DropTable")));

    EXPECT_FALSE(agentInfoPersistance->ResetToDefault());
}

TEST_F(AgentInfoPersistanceTest, TestResetToDefaultDropTableAgentGroupFails)
{
    EXPECT_CALL(*mockPersistence, DropTable("agent_info")).Times(1);
    EXPECT_CALL(*mockPersistence, DropTable("agent_group"))
        .WillOnce(testing::Throw(std::runtime_error("Error DropTable")));

    EXPECT_FALSE(agentInfoPersistance->ResetToDefault());
}

TEST_F(AgentInfoPersistanceTest, TestResetToDefaultCreateAgentInfoTableFails)
{
    EXPECT_CALL(*mockPersistence, DropTable("agent_info")).Times(1);
    EXPECT_CALL(*mockPersistence, DropTable("agent_group")).Times(1);
    EXPECT_CALL(*mockPersistence, CreateTable("agent_info", testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error CreateAgentInfoTable")));

    EXPECT_FALSE(agentInfoPersistance->ResetToDefault());
}

TEST_F(AgentInfoPersistanceTest, TestResetToDefaultCreateAgentGroupTableFails)
{
    EXPECT_CALL(*mockPersistence, DropTable("agent_info")).Times(1);
    EXPECT_CALL(*mockPersistence, DropTable("agent_group")).Times(1);
    EXPECT_CALL(*mockPersistence, CreateTable("agent_info", testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, CreateTable("agent_group", testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error CreateAgentGroupTable")));

    EXPECT_FALSE(agentInfoPersistance->ResetToDefault());
}

TEST_F(AgentInfoPersistanceTest, TestResetToDefaultInsertFails)
{
    EXPECT_CALL(*mockPersistence, DropTable("agent_info")).Times(1);
    EXPECT_CALL(*mockPersistence, DropTable("agent_group")).Times(1);
    EXPECT_CALL(*mockPersistence, CreateTable("agent_info", testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, CreateTable("agent_group", testing::_)).Times(1);
    EXPECT_CALL(*mockPersistence, GetCount("agent_info", testing::_, testing::_)).WillOnce(testing::Return(0));
    EXPECT_CALL(*mockPersistence, Insert("agent_info", testing::_))
        .WillOnce(testing::Throw(std::runtime_error("Error Insert")));

    EXPECT_FALSE(agentInfoPersistance->ResetToDefault());
}

int main(int argc, char** argv)
{
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
