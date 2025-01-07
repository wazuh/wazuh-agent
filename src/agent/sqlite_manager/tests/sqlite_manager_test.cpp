#include <gtest/gtest.h>

#include <sqlite_manager.hpp>

#include <memory>
#include <string>
#include <vector>

using namespace sqlite_manager;

class SQLiteManagerTest : public ::testing::Test
{
protected:
    const std::string m_dbName = "testdb.db";
    const std::string m_tableName = "TestTable";

    std::unique_ptr<SQLiteManager> m_db;

    void SetUp() override
    {
        m_db = std::make_unique<SQLiteManager>(m_dbName);
    }

    void TearDown() override {}

    void AddTestData()
    {
        EXPECT_NO_THROW(m_db->Remove(m_tableName));
        m_db->Insert(m_tableName,
                     {Column("Name", ColumnType::TEXT, "ItemName"), Column("Status", ColumnType::TEXT, "ItemStatus")});
        m_db->Insert(
            m_tableName,
            {Column("Name", ColumnType::TEXT, "MyTestName"), Column("Status", ColumnType::TEXT, "MyTestValue")});
        m_db->Insert(
            m_tableName,
            {Column("Name", ColumnType::TEXT, "ItemName2"), Column("Status", ColumnType::TEXT, "ItemStatus2")});
        m_db->Insert(m_tableName,
                     {Column("Name", ColumnType::TEXT, "ItemName3"),
                      Column("Status", ColumnType::TEXT, "ItemStatus3"),
                      Column("Module", ColumnType::TEXT, "ItemModule3")});
        m_db->Insert(m_tableName,
                     {Column("Name", ColumnType::TEXT, "ItemName4"),
                      Column("Status", ColumnType::TEXT, "ItemStatus4"),
                      Column("Module", ColumnType::TEXT, "ItemModule4"),
                      Column("Orden", ColumnType::INTEGER, "19"),
                      Column("Amount", ColumnType::REAL, "2.8")});
        m_db->Insert(m_tableName,
                     {Column("Name", ColumnType::TEXT, "ItemName5"),
                      Column("Status", ColumnType::TEXT, "ItemStatus5"),
                      Column("Module", ColumnType::TEXT, "ItemModule5"),
                      Column("Orden", ColumnType::INTEGER, "21"),
                      Column("Amount", ColumnType::REAL, "3.5")});
    }
};

TEST_F(SQLiteManagerTest, CreateTableTest)
{
    Column col1 {"Id", ColumnType::INTEGER, true, true, true};
    Column col2 {"Name", ColumnType::TEXT, true, false, false};
    Column col3 {"Status", ColumnType::TEXT, true, false};
    Column col4 {"Module", ColumnType::TEXT, false, false};
    Column col5 {"Orden", ColumnType::INTEGER, false, false, false};
    Column col6 {"Amount", ColumnType::REAL, false, false, false};
    EXPECT_NO_THROW(m_db->CreateTable(m_tableName, {col1, col2, col3, col4, col5, col6}));
    EXPECT_TRUE(m_db->TableExists(m_tableName));

    Column col21 {"Id", ColumnType::INTEGER, true, false, true};
    Column col212 {"Id2", ColumnType::INTEGER, true, false, true};
    Column col22 {"Name", ColumnType::TEXT, true, false, true};
    Column col23 {"Status", ColumnType::TEXT, true, false};
    Column col24 {"Module", ColumnType::TEXT, false, false};
    Column col25 {"Orden", ColumnType::INTEGER, false, false, false};
    Column col26 {"Amount", ColumnType::REAL, false, false, false};
    EXPECT_NO_THROW(m_db->CreateTable("TableTest2", {col21, col212, col22, col23, col24, col25, col26}));
    EXPECT_TRUE(m_db->TableExists("TableTest2"));
}

TEST_F(SQLiteManagerTest, InsertTest)
{
    Column col1 {"Name", ColumnType::TEXT, "ItemName1"};
    Column col2 {"Status", ColumnType::TEXT, "ItemStatus1"};

    EXPECT_NO_THROW(m_db->Insert(m_tableName, {col1, col2}));
    EXPECT_NO_THROW(m_db->Insert(
        m_tableName,
        {Column("Name", ColumnType::TEXT, "ItemName2"), Column("Status", ColumnType::TEXT, "ItemStatus2")}));
    EXPECT_NO_THROW(m_db->Insert(m_tableName,
                                 {Column("Name", ColumnType::TEXT, "ItemName3"),
                                  Column("Status", ColumnType::TEXT, "ItemStatus3"),
                                  Column("Module", ColumnType::TEXT, "ItemModule3")}));

    EXPECT_NO_THROW(m_db->Insert(m_tableName,
                                 {Column("Name", ColumnType::TEXT, "ItemName4"),
                                  Column("Status", ColumnType::TEXT, "ItemStatus4"),
                                  Column("Module", ColumnType::TEXT, "ItemModule4"),
                                  Column("Orden", ColumnType::INTEGER, "16")}));

    EXPECT_NO_THROW(m_db->Insert(m_tableName,
                                 {Column("Name", ColumnType::TEXT, "ItemName4"),
                                  Column("Status", ColumnType::TEXT, "ItemStatus4"),
                                  Column("Module", ColumnType::TEXT, "ItemModule4"),
                                  Column("Orden", ColumnType::INTEGER, "16"),
                                  Column("Amount", ColumnType::REAL, "4.5")}));
}

TEST_F(SQLiteManagerTest, GetCountTest)
{
    EXPECT_NO_THROW(m_db->Remove(m_tableName));
    int count = m_db->GetCount(m_tableName);
    EXPECT_EQ(count, 0);

    Column col1 {"Name", ColumnType::TEXT, "ItemName1"};
    Column col2 {"Status", ColumnType::TEXT, "ItemStatus1"};
    EXPECT_NO_THROW(m_db->Insert(m_tableName, {col1, col2}));

    count = m_db->GetCount(m_tableName);
    EXPECT_EQ(count, 1);

    EXPECT_NO_THROW(m_db->Insert(m_tableName, {col1, col2}));

    count = m_db->GetCount(m_tableName);
    EXPECT_EQ(count, 2);
}

static void DumpResults(std::vector<Row>& ret)
{
    std::cout << "---------- " << ret.size() << " rows returned. ----------" << '\n';
    for (const auto& row : ret)
    {
        for (const auto& field : row)
        {
            std::cout << "[" << field.Name << ": " << field.Value << "]";
        }
        std::cout << '\n';
    }
}

TEST_F(SQLiteManagerTest, SelectTest)
{
    AddTestData();

    std::vector<Column> cols;

    // all fields, no selection criteria
    std::vector<Row> ret = m_db->Select(m_tableName, cols);

    DumpResults(ret);
    EXPECT_NE(ret.size(), 0);

    // all fields with default selection criteria
    ret = m_db->Select(
        m_tableName,
        cols,
        {Column("Name", ColumnType::TEXT, "MyTestName"), Column("Status", ColumnType::TEXT, "MyTestValue")});

    DumpResults(ret);
    EXPECT_NE(ret.size(), 0);

    // all fields with 'OR' selection criteria
    ret = m_db->Select(
        m_tableName,
        cols,
        {Column("Name", ColumnType::TEXT, "MyTestName"), Column("Module", ColumnType::TEXT, "ItemModule5")},
        LogicalOperator::OR);

    DumpResults(ret);
    EXPECT_EQ(ret.size(), 2);

    // only Name field no selection criteria
    cols.clear();
    cols.emplace_back("Name", ColumnType::TEXT, "MyTestName");
    ret.clear();
    ret = m_db->Select(m_tableName, cols);

    DumpResults(ret);
    EXPECT_NE(ret.size(), 0);

    // only Name field with default selection criteria
    cols.clear();
    cols.emplace_back("Name", ColumnType::TEXT, "MyTestName");
    ret.clear();
    ret = m_db->Select(
        m_tableName,
        cols,
        {Column("Name", ColumnType::TEXT, "MyTestName"), Column("Status", ColumnType::TEXT, "MyTestValue")});

    DumpResults(ret);
    EXPECT_NE(ret.size(), 0);

    // only Name field with single selection criteria
    cols.clear();
    cols.emplace_back("Name", ColumnType::TEXT, "MyTestName");
    ret.clear();
    ret = m_db->Select(m_tableName, cols, {Column("Amount", ColumnType::REAL, "3.5")});

    DumpResults(ret);
    EXPECT_EQ(ret.size(), 1);

    // only Name and Amount fields with single selection criteria
    cols.clear();
    cols.emplace_back("Name", ColumnType::TEXT, "MyTestName");
    cols.emplace_back("Amount", ColumnType::REAL, "Amount");
    ret.clear();
    ret = m_db->Select(m_tableName, cols, {Column("Amount", ColumnType::REAL, "2.8")});

    DumpResults(ret);
    EXPECT_EQ(ret.size(), 1);
}

TEST_F(SQLiteManagerTest, RemoveTest)
{
    AddTestData();

    int initialCount = m_db->GetCount(m_tableName);

    // Remove a single record
    EXPECT_NO_THROW(m_db->Remove(
        m_tableName,
        {Column("Name", ColumnType::TEXT, "MyTestName"), Column("Status", ColumnType::TEXT, "MyTestValue")}));
    int count = m_db->GetCount(m_tableName);
    EXPECT_EQ(count, initialCount - 1);

    // Remove all remaining records
    EXPECT_NO_THROW(m_db->Remove(m_tableName));
    count = m_db->GetCount(m_tableName);
    EXPECT_EQ(count, 0);
}

TEST_F(SQLiteManagerTest, UpdateTest)
{
    AddTestData();
    EXPECT_NO_THROW(m_db->Update(
        m_tableName,
        {Column("Name", ColumnType::TEXT, "Updated name"), Column("Status", ColumnType::TEXT, "Updated status")},
        {Column("Name", ColumnType::TEXT, "MyTestName")}));

    auto ret = m_db->Select(m_tableName, {}, {Column("Name", ColumnType::TEXT, "Updated name")});
    DumpResults(ret);
    EXPECT_EQ(ret.size(), 1);

    EXPECT_NO_THROW(m_db->Update(
        m_tableName,
        {Column("Name", ColumnType::TEXT, "Updated name2"), Column("Status", ColumnType::TEXT, "Updated status2")},
        {Column("Name", ColumnType::TEXT, "Updated name"), Column("Status", ColumnType::TEXT, "Updated status")}));

    ret = m_db->Select(m_tableName, {}, {Column("Name", ColumnType::TEXT, "Updated name2")});
    DumpResults(ret);
    EXPECT_EQ(ret.size(), 1);

    EXPECT_NO_THROW(m_db->Update(m_tableName,
                                 {Column("Amount", ColumnType::REAL, "2.0")},
                                 {Column("Name", ColumnType::TEXT, "Updated name2"),
                                  Column("Status", ColumnType::TEXT, "ItemStatus3"),
                                  Column("Status", ColumnType::TEXT, "Updated status3")},
                                 LogicalOperator::OR));

    ret = m_db->Select(m_tableName, {}, {});
    DumpResults(ret);

    EXPECT_NO_THROW(m_db->Update(m_tableName,
                                 {Column("Amount", ColumnType::REAL, "2.0")},
                                 {Column("Status", ColumnType::TEXT, "ItemStatus3")},
                                 LogicalOperator::OR));

    ret = m_db->Select(m_tableName, {}, {});
    DumpResults(ret);

    EXPECT_NO_THROW(m_db->Update(m_tableName, {Column("Amount", ColumnType::REAL, "2.0")}, {}));

    ret = m_db->Select(m_tableName, {}, {});
    DumpResults(ret);
}

TEST_F(SQLiteManagerTest, TransactionTest)
{
    {
        auto transaction = m_db->BeginTransaction();

        m_db->Insert(m_tableName,
                     {Column("Name", ColumnType::TEXT, "TransactionName"),
                      Column("Status", ColumnType::TEXT, "TransactionStatus")});

        m_db->Insert(m_tableName,
                     {Column("Name", ColumnType::TEXT, "TransactionName2"),
                      Column("Status", ColumnType::TEXT, "TransactionStatus2")});
        EXPECT_NO_THROW(m_db->RollbackTransaction(transaction));
    }

    // since we rolled back the transaction we should find nothing
    auto ret = m_db->Select(m_tableName, {}, {Column("Status", ColumnType::TEXT, "TransactionStatus2")});

    EXPECT_EQ(ret.size(), 0);

    {
        auto transaction = m_db->BeginTransaction();

        m_db->Insert(m_tableName,
                     {Column("Name", ColumnType::TEXT, "TransactionName"),
                      Column("Status", ColumnType::TEXT, "TransactionStatus")});

        m_db->Insert(m_tableName,
                     {Column("Name", ColumnType::TEXT, "TransactionName2"),
                      Column("Status", ColumnType::TEXT, "TransactionStatus2")});
    }

    // since transaction obejct ran out of scope without being committed we should find nothing
    ret = m_db->Select(m_tableName, {}, {Column("Status", ColumnType::TEXT, "TransactionStatus2")});

    EXPECT_EQ(ret.size(), 0);

    {
        auto transaction = m_db->BeginTransaction();

        m_db->Insert(m_tableName,
                     {Column("Name", ColumnType::TEXT, "TransactionName"),
                      Column("Status", ColumnType::TEXT, "TransactionStatus")});

        m_db->Insert(m_tableName,
                     {Column("Name", ColumnType::TEXT, "TransactionName2"),
                      Column("Status", ColumnType::TEXT, "TransactionStatus2")});
        EXPECT_NO_THROW(m_db->CommitTransaction(transaction));
    }

    // since we commited the transaction we should find something
    ret = m_db->Select(m_tableName, {}, {Column("Status", ColumnType::TEXT, "TransactionStatus2")});

    EXPECT_EQ(ret.size(), 1);
}

TEST_F(SQLiteManagerTest, DropTableTest)
{
    Column col1 {"Id", ColumnType::INTEGER, true, true, true};
    Column col2 {"Name", ColumnType::TEXT, true, false};
    Column col3 {"Status", ColumnType::TEXT, true, false};
    Column col4 {"Module", ColumnType::TEXT, false, false};
    Column col5 {"Orden", ColumnType::INTEGER, false, false, false};

    EXPECT_NO_THROW(m_db->CreateTable("DropMe", {col1, col2, col3, col4, col5}));
    EXPECT_TRUE(m_db->TableExists("DropMe"));
    EXPECT_NO_THROW(m_db->DropTable("DropMe"));
    EXPECT_FALSE(m_db->TableExists("DropMe"));

    EXPECT_ANY_THROW(auto ret = m_db->Select("DropMe", {}, {}));
}
