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
