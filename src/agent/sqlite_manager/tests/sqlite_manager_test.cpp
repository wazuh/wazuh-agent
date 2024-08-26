#include "gtest/gtest.h"

class SQLiteManagerTest : public ::testing::Test
{
protected:
    const std::string dbName = "testdb.db";

    void SetUp() override {}

    void TearDown() override {}
};

TEST_F(SQLiteManagerTest, OneTest)
{
    EXPECT_EQ(2, 2);
}
