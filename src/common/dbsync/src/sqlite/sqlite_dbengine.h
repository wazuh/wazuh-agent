#pragma once

#include "db_exception.h"
#include "dbengine.h"
#include "isqliteWrapper.hpp"
#include "mapWrapperSafe.hpp"
#include "sqliteWrapperFactory.hpp"
#include <iostream>
#include <mutex>
#include <queue>
#include <tuple>

constexpr auto TEMP_TABLE_SUBFIX {"_TEMP"};

constexpr auto STATUS_FIELD_NAME {"db_status_field_dm"};
constexpr auto STATUS_FIELD_TYPE {"INTEGER"};

constexpr auto CACHE_STMT_LIMIT {30ull};

const std::vector<std::string> InternalColumnNames = {{STATUS_FIELD_NAME}};

/// @brief Column types
enum ColumnType
{
    Unknown = 0,
    Text,
    Integer,
    BigInt,
    UnsignedBigInt,
    Double,
    Blob,
};

const std::map<std::string, ColumnType> ColumnTypeNames = {
    {"UNKNOWN", Unknown},
    {"TEXT", Text},
    {"INTEGER", Integer},
    {"BIGINT", BigInt},
    {"UNSIGNED BIGINT", UnsignedBigInt},
    {"DOUBLE", Double},
    {"BLOB", Blob},
};

/// @brief Table headers
enum TableHeader
{
    CID = 0,
    Name,
    Type,
    PK,
    TXNStatusField
};

/// @brief Generic tuple index
enum GenericTupleIndex
{
    GenType = 0,
    GenString,
    GenInteger,
    GenBigInt,
    GenUnsignedBigInt,
    GenDouble
};

using ColumnData = std::tuple<int32_t, std::string, ColumnType, bool, bool>;

using TableColumns = std::vector<ColumnData>;

using TableField = std::tuple<int32_t,                    // Type
                              std::optional<std::string>, // Text or null
                              std::optional<int32_t>,     // Integer or null
                              std::optional<int64_t>,     // BigInt or null
                              std::optional<uint64_t>,    // UnsignedBigInt or null
                              std::optional<double_t>>;   // Double or null

using Row = std::map<std::string, TableField>;

using Field = std::pair<const std::string, TableField>;

/// @brief Response types
enum ResponseType
{
    RTJson = 0,
    RTCallback
};

/// @brief DbEngine exception
class dbengine_error : public DbSync::dbsync_error
{
public:
    /// @brief Constructor
    /// @param exceptionInfo error information
    explicit dbengine_error(const std::pair<int, std::string>& exceptionInfo)
        : DbSync::dbsync_error {exceptionInfo.first, "dbEngine: " + exceptionInfo.second}
    {
    }
};

/// @brief Max rows
struct MaxRows final
{
    int64_t maxRows;
    int64_t currentRows;
};

/// @brief SQLiteDBEngine
class SQLiteDBEngine final : public DbSync::IDbEngine
{
public:
    /// @brief Constructor
    /// @param sqliteFactory sqlite factory
    /// @param path database path
    /// @param tableStmtCreation SQL sentence to create tables in a SQL engine
    /// @param dbManagement Database management type to be used at startup
    /// @param upgradeStatements SQL sentences to be executed when upgrading the database
    SQLiteDBEngine(const std::shared_ptr<SQLiteLegacy::ISQLiteFactory>& sqliteFactory,
                   const std::string& path,
                   const std::string& tableStmtCreation,
                   const DbManagement dbManagement = DbManagement::VOLATILE,
                   const std::vector<std::string>& upgradeStatements = {});

    /// @brief Destructor
    ~SQLiteDBEngine();

    /// @brief Inserts data into a table
    /// @param table table name
    /// @param data data to be inserted
    void bulkInsert(const std::string& table, const nlohmann::json& data) override;

    /// @brief Refreshes table data
    /// @param data data
    /// @param callback callback
    /// @param lock lock
    void refreshTableData(const nlohmann::json& data,
                          const DbSync::ResultCallback& callback,
                          std::unique_lock<std::shared_timed_mutex>& lock) override;

    /// @brief Syncs table row data
    /// @param jsInput JSON input
    /// @param callback callback
    /// @param inTransaction transaction
    /// @param lock mutex
    void syncTableRowData(const nlohmann::json& jsInput,
                          const DbSync::ResultCallback& callback,
                          const bool inTransaction,
                          ILocking& lock) override;

    /// @brief Sets the maximum number of rows for a table
    /// @param table table name
    /// @param maxRows maximum number of rows
    void setMaxRows(const std::string& table, const int64_t maxRows) override;

    /// @brief Sets the status field for a table
    /// @param tableNames table names
    void initializeStatusField(const nlohmann::json& tableNames) override;

    /// @brief Deletes rows by status field
    /// @param tableNames table names
    void deleteRowsByStatusField(const nlohmann::json& tableNames) override;

    /// @brief Returns rows marked for delete
    /// @param tableNames table names
    /// @param callback callback
    /// @param lock lock
    void returnRowsMarkedForDelete(const nlohmann::json& tableNames,
                                   const DbSync::ResultCallback& callback,
                                   std::unique_lock<std::shared_timed_mutex>& lock) override;

    /// @brief Selects data from a table
    /// @param table table name
    /// @param query query
    /// @param callback callback
    /// @param lock lock
    void selectData(const std::string& table,
                    const nlohmann::json& query,
                    const DbSync::ResultCallback& callback,
                    std::unique_lock<std::shared_timed_mutex>& lock) override;

    /// @brief Deletes table rows data
    /// @param table table name
    /// @param jsDeletionData JSON deletion data
    void deleteTableRowsData(const std::string& table, const nlohmann::json& jsDeletionData) override;

    /// @brief Adds table relationship
    /// @param data JSON data
    void addTableRelationship(const nlohmann::json& data) override;

private:
    /// @brief Delete copy constructor
    SQLiteDBEngine(const SQLiteDBEngine&) = delete;

    /// @brief Delete copy operator
    SQLiteDBEngine& operator=(const SQLiteDBEngine&) = delete;

    /// @brief Initializes the database
    /// @param path database path
    /// @param tableStmtCreation SQL sentence to create tables in a SQL engine
    /// @param dbManagement Database management type to be used at startup
    /// @param upgradeStatements SQL sentences to be executed when upgrading the database
    void initialize(const std::string& path,
                    const std::string& tableStmtCreation,
                    const DbManagement dbManagement,
                    const std::vector<std::string>& upgradeStatements);

    /// @brief Cleans the database
    /// @param path database path
    /// @return true if the database was cleaned
    bool cleanDB(const std::string& path);

    /// @brief Returns the database version
    /// @return database version
    size_t getDbVersion();

    /// @brief Loads the table data
    /// @param table table name
    /// @return number of rows
    size_t loadTableData(const std::string& table);

    /// @brief Loads the field data
    /// @param table table name
    /// @return true if the data was loaded
    bool loadFieldData(const std::string& table);

    /// @brief Builds the insert data SQL query
    /// @param table table name
    /// @param data data
    /// @return insert data SQL query
    std::string buildInsertDataSqlQuery(const std::string& table, const nlohmann::json& data = {});

    /// @brief Builds the delete bulk data SQL query
    /// @param table table name
    /// @param primaryKeyList primary key list
    /// @return delete bulk data SQL query
    std::string buildDeleteBulkDataSqlQuery(const std::string& table, const std::vector<std::string>& primaryKeyList);

    /// @brief Builds the select query
    /// @param table table name
    /// @param jsQuery query
    /// @return select query
    std::string buildSelectQuery(const std::string& table, const nlohmann::json& jsQuery);

    /// @brief Returns the column type name
    /// @param type column type
    /// @return column type name
    ColumnType columnTypeName(const std::string& type);

    /// @brief Binds the JSON data
    /// @param stmt statement
    /// @param cd column data
    /// @param valueType JSON value type
    /// @param cid column index
    /// @return true if the data was bound
    bool bindJsonData(const std::shared_ptr<SQLiteLegacy::IStatement> stmt,
                      const ColumnData& cd,
                      const nlohmann::json::value_type& valueType,
                      const int32_t cid);

    /// @brief Creates the copy temp table
    /// @param table table name
    /// @return true if the table was created
    bool createCopyTempTable(const std::string& table);

    /// @brief Returns the table create query
    /// @param table table name
    /// @param resultQuery query result
    /// @return true if the query was returned
    bool getTableCreateQuery(const std::string& table, std::string& resultQuery);

    /// @brief Returns the primary keys from the table
    /// @param table table name
    /// @param primaryKeyList primary key list
    /// @return true if the primary keys were returned
    bool getPrimaryKeysFromTable(const std::string& table, std::vector<std::string>& primaryKeyList);

    /// @brief Removes the not exists rows
    /// @param table table name
    /// @param primaryKeyList primary key list
    /// @param callback callback
    /// @param lock lock
    /// @return true if the rows were removed
    bool removeNotExistsRows(const std::string& table,
                             const std::vector<std::string>& primaryKeyList,
                             const DbSync::ResultCallback& callback,
                             std::unique_lock<std::shared_timed_mutex>& lock);

    /// @brief Returns the row diff
    /// @param primaryKeyList primary key list
    /// @param ignoredColumns ignored columns
    /// @param table table name
    /// @param data data
    /// @param updatedData updated data
    /// @param oldData old data
    /// @return true if the row diff was returned
    bool getRowDiff(const std::vector<std::string>& primaryKeyList,
                    const nlohmann::json& ignoredColumns,
                    const std::string& table,
                    const nlohmann::json& data,
                    nlohmann::json& updatedData,
                    nlohmann::json& oldData);

    /// @brief Inserts the new rows
    /// @param table table name
    /// @param primaryKeyList primary key list
    /// @param callback callback
    /// @param lock lock
    /// @return true if the rows were inserted
    bool insertNewRows(const std::string& table,
                       const std::vector<std::string>& primaryKeyList,
                       const DbSync::ResultCallback& callback,
                       std::unique_lock<std::shared_timed_mutex>& lock);

    /// @brief Deletes the rows
    /// @param table table name
    /// @param primaryKeyList primary key list
    /// @param rowsToRemove rows to remove
    /// @return true if the rows were deleted
    bool deleteRows(const std::string& table,
                    const std::vector<std::string>& primaryKeyList,
                    const std::vector<Row>& rowsToRemove);

    /// @brief Deletes the rows by data
    /// @param table table name
    /// @param data data to delete
    /// @param primaryKeyList primary key list
    void
    deleteRows(const std::string& table, const nlohmann::json& data, const std::vector<std::string>& primaryKeyList);

    /// @brief Deletes the rows by primary key
    /// @param table table name
    /// @param data data to delete
    void deleteRowsbyPK(const std::string& table, const nlohmann::json& data);

    /// @brief Gets the table data
    /// @param stmt statement
    /// @param index index
    /// @param type column type
    /// @param fieldName field name
    /// @param row row
    void getTableData(const std::shared_ptr<SQLiteLegacy::IStatement> stmt,
                      const int32_t index,
                      const ColumnType& type,
                      const std::string& fieldName,
                      Row& row);

    /// @brief Binds the field data
    /// @param stmt statement
    /// @param index index
    /// @param fieldData field data
    void bindFieldData(const std::shared_ptr<SQLiteLegacy::IStatement> stmt,
                       const int32_t index,
                       const TableField& fieldData);

    /// @brief Returns the left only query
    /// @param t1 first value
    /// @param t2 second value
    /// @param primaryKeyList primary key list
    /// @param returnOnlyPKFields return only primary key fields
    /// @return left only query
    std::string buildLeftOnlyQuery(const std::string& t1,
                                   const std::string& t2,
                                   const std::vector<std::string>& primaryKeyList,
                                   const bool returnOnlyPKFields = false);

    /// @brief Returns the left only
    /// @param t1 first value
    /// @param t2 second value
    /// @param primaryKeyList primary key list
    /// @param returnRows return rows
    /// @return true if the left only was returned
    bool getLeftOnly(const std::string& t1,
                     const std::string& t2,
                     const std::vector<std::string>& primaryKeyList,
                     std::vector<Row>& returnRows);

    /// @brief Returns pk list left only
    /// @param t1 first value
    /// @param t2 second value
    /// @param primaryKeyList primary key list
    /// @param returnRows return rows
    /// @return true if the pk list left only was returned
    bool getPKListLeftOnly(const std::string& t1,
                           const std::string& t2,
                           const std::vector<std::string>& primaryKeyList,
                           std::vector<Row>& returnRows);

    /// @brief Inserts the data
    /// @param table table name
    /// @param data data to insert
    void bulkInsert(const std::string& table, const std::vector<Row>& data);

    /// @brief Deletes the temp table
    /// @param table table name
    void deleteTempTable(const std::string& table);

    /// @brief Builds the modified rows query
    /// @param t1 first table
    /// @param t2 second table
    /// @param primaryKeyList primary key list
    /// @return modified rows query
    std::string buildModifiedRowsQuery(const std::string& t1,
                                       const std::string& t2,
                                       const std::vector<std::string>& primaryKeyList);

    /// @brief Changes the modified rows
    /// @param table table name
    /// @param primaryKeyList primary key list
    /// @param callback callback
    /// @param lock lock
    /// @return number of modified rows
    int changeModifiedRows(const std::string& table,
                           const std::vector<std::string>& primaryKeyList,
                           const DbSync::ResultCallback& callback,
                           std::unique_lock<std::shared_timed_mutex>& lock);

    /// @brief Builds the select matching pks sql query
    /// @param table table name
    /// @param primaryKeyList primary key list
    /// @return select matching pks sql query
    std::string buildSelectMatchingPKsSqlQuery(const std::string& table,
                                               const std::vector<std::string>& primaryKeyList);

    /// @brief Builds the update data sql query
    /// @param table table name
    /// @param primaryKeyList primary key list
    /// @param row row
    /// @param field field
    /// @return update data sql query
    std::string buildUpdateDataSqlQuery(const std::string& table,
                                        const std::vector<std::string>& primaryKeyList,
                                        const Row& row,
                                        const std::pair<const std::string, TableField>& field);

    /// @brief Builds the update partial data sql query
    /// @param table table name
    /// @param data data
    /// @param primaryKeyList primary key list
    /// @return update partial data sql query
    std::string buildUpdatePartialDataSqlQuery(const std::string& table,
                                               const nlohmann::json& data,
                                               const std::vector<std::string>& primaryKeyList);

    /// @brief Gets the rows to modify
    /// @param table table name
    /// @param primaryKeyList primary key list
    /// @param rowKeysValue row keys value
    /// @return true if the rows to modify were returned
    bool getRowsToModify(const std::string& table,
                         const std::vector<std::string>& primaryKeyList,
                         std::vector<Row>& rowKeysValue);

    /// @brief Updates the single row
    /// @param table table name
    /// @param jsData json data
    void updateSingleRow(const std::string& table, const nlohmann::json& jsData);

    /// @brief Updates the rows
    /// @param table table name
    /// @param primaryKeyList primary key list
    /// @param rowKeysValue row keys value
    /// @return true if the rows were updated
    bool updateRows(const std::string& table,
                    const std::vector<std::string>& primaryKeyList,
                    const std::vector<Row>& rowKeysValue);

    /// @brief Gets the field value from tuple
    /// @param value value
    /// @param resultValue result value
    /// @param quotationMarks quotation marks
    void getFieldValueFromTuple(const Field& value, std::string& resultValue, const bool quotationMarks = false);

    /// @brief Gets the field value from tuple
    /// @param value value
    /// @param object json object
    void getFieldValueFromTuple(const Field& value, nlohmann::json& object);

    /// @brief Get a statement
    /// @param sql SQL sentence
    /// @return statement
    std::shared_ptr<SQLiteLegacy::IStatement> getStatement(const std::string& sql);

    /// @brief Gets the select all query
    /// @param table table name
    /// @param tableFields table fields
    /// @return select all query
    std::string getSelectAllQuery(const std::string& table, const TableColumns& tableFields) const;

    /// @brief Builds the delete relation trigger
    /// @param data data
    /// @param baseTable base table
    /// @return delete relation trigger
    std::string buildDeleteRelationTrigger(const nlohmann::json& data, const std::string& baseTable);

    /// @brief Builds the update relation trigger
    /// @param data data
    /// @param baseTable base table
    /// @param primaryKeys primary keys
    /// @return update relation trigger
    std::string buildUpdateRelationTrigger(const nlohmann::json& data,
                                           const std::string& baseTable,
                                           const std::vector<std::string>& primaryKeys);

    /// @brief Updates the table row counter
    /// @param table table name
    /// @param rowModifyCount row modify count
    void updateTableRowCounter(const std::string& table, const long long rowModifyCount);

    /// @brief Inserts the element
    /// @param table table name
    /// @param tableColumns table columns
    /// @param element element
    /// @param callback callback
    void InsertElement(const std::string& table,
                       const TableColumns& tableColumns,
                       const nlohmann::json& element,
                       const std::function<void()>& callback = {});

    Utils::MapWrapperSafe<std::string, TableColumns> m_tableFields;
    std::deque<std::pair<std::string, std::shared_ptr<SQLiteLegacy::IStatement>>> m_statementsCache;
    const std::shared_ptr<SQLiteLegacy::ISQLiteFactory> m_sqliteFactory;
    std::shared_ptr<SQLiteLegacy::IConnection> m_sqliteConnection;
    std::mutex m_stmtMutex;
    std::unique_ptr<SQLiteLegacy::ITransaction> m_transaction;
    std::mutex m_maxRowsMutex;
    std::map<std::string, MaxRows> m_maxRows;
};
