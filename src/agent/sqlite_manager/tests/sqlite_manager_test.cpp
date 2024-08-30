#include "gtest/gtest.h"

#include <sqlite_manager.hpp>

class SQLiteManagerTest : public ::testing::Test
{
protected:
    const std::string m_dbName = "testdb.db";
    const std::string m_tableName = "TestTable";

    std::unique_ptr<sqlite_manager::SQLiteManager> m_db;

    void SetUp() override
    {
        m_db = std::make_unique<sqlite_manager::SQLiteManager>(m_dbName);
    }

    void TearDown() override {}
};

TEST_F(SQLiteManagerTest, CreateTableTest)
{
    sqlite_manager::Col col1 {"Id", sqlite_manager::ColumnType::INTEGER, true, true, true};
    sqlite_manager::Col col2 {"Name", sqlite_manager::ColumnType::TEXT, true, false};
    sqlite_manager::Col col3 {"Status", sqlite_manager::ColumnType::TEXT, true, false};

    EXPECT_NO_THROW(m_db->CreateTable(m_tableName, {col1, col2, col3}));
}

TEST_F(SQLiteManagerTest, InsertTest)
{
    sqlite_manager::Col col1 {"Name", sqlite_manager::ColumnType::TEXT, "ItemName1"};
    sqlite_manager::Col col2 {"Status", sqlite_manager::ColumnType::TEXT, "ItemStatus1"};

    EXPECT_NO_THROW(m_db->Insert(m_tableName, {col1, col2}));
    EXPECT_NO_THROW(m_db->Insert(m_tableName,
                                 {sqlite_manager::Col("Name", sqlite_manager::ColumnType::TEXT, "ItemName2"),
                                  sqlite_manager::Col("Status", sqlite_manager::ColumnType::TEXT, "ItemStatus2")}));
}

TEST_F(SQLiteManagerTest, GetCountTest)
{
    m_db->ExecuteNoSelectSQL("DELETE FROM TestTable");
    int count = m_db->GetCount(m_tableName);
    EXPECT_EQ(count, 0);

    sqlite_manager::Col col1 {"Name", sqlite_manager::ColumnType::TEXT, "ItemName1"};
    sqlite_manager::Col col2 {"Status", sqlite_manager::ColumnType::TEXT, "ItemStatus1"};
    EXPECT_NO_THROW(m_db->Insert(m_tableName, {col1, col2}));

    count = m_db->GetCount(m_tableName);
    EXPECT_EQ(count, 1);

    EXPECT_NO_THROW(m_db->Insert(m_tableName, {col1, col2}));

    count = m_db->GetCount(m_tableName);
    EXPECT_EQ(count, 2);
}
