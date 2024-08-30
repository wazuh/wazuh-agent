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

TEST_F(SQLiteManagerTest, OneTest)
{
    EXPECT_EQ(2, 2);
}
