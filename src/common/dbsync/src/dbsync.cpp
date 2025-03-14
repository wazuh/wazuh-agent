#include "dbsync.h"
#include "cjsonSmartDeleter.hpp"
#include "dbsync.hpp"
#include "dbsyncPipelineFactory.h"
#include "dbsync_implementation.h"
#include <map>
#include <mutex>

#ifdef __cplusplus
extern "C"
{
#endif

    using namespace DbSync;

    static std::function<void(const std::string&)> GS_LOG_FUNCTION;

    static void LogMessage(const std::string& msg)
    {
        if (!msg.empty() && GS_LOG_FUNCTION)
        {
            GS_LOG_FUNCTION(msg);
        }
    }

    void dbsync_initialize(log_fnc_t logFunction)
    {
        DBSync::initialize([logFunction](const std::string& msg) { logFunction(msg.c_str()); });
    }

    DBSYNC_HANDLE _dbsync_create(const HostType hostType,
                                 const DbEngineType dbType,
                                 const char* path,
                                 const char* sqlStatement,
                                 const DbManagement dbManagement,
                                 const char** upgradeStatements)
    {
        DBSYNC_HANDLE retVal {nullptr};
        std::string errorMessage;

        if (!path || !sqlStatement)
        {
            errorMessage += "Invalid path or sqlStatement.";
        }
        else
        {
            try
            {
                auto upgrade_statements = std::vector<std::string>();

                while (upgradeStatements && *upgradeStatements)
                {
                    upgrade_statements.emplace_back(*upgradeStatements);
                    upgradeStatements++; // NOLINT(cppcoreguidelines-pro-bounds-pointer-arithmetic)
                }

                retVal = DBSyncImplementation::instance().initialize(
                    hostType, dbType, path, sqlStatement, dbManagement, upgrade_statements);
            }
            catch (const DbSync::dbsync_error& ex)
            {
                errorMessage += "DB error, id: " + std::to_string(ex.id()) + ". " + ex.what();
            }
            catch (...) // NOLINT(bugprone-empty-catch) // NOLINT(bugprone-empty-catch)
            {
                errorMessage += "Unrecognized error.";
            }
        }

        LogMessage(errorMessage);
        return retVal;
    }

    DBSYNC_HANDLE
    dbsync_create(const HostType hostType, const DbEngineType dbType, const char* path, const char* sqlStatement)
    {
        return _dbsync_create(hostType, dbType, path, sqlStatement, DbManagement::VOLATILE, nullptr);
    }

    DBSYNC_HANDLE dbsync_create_persistent(const HostType hostType,
                                           const DbEngineType dbType,
                                           const char* path,
                                           const char* sqlStatement,
                                           const char** upgradeStatements)
    {
        return _dbsync_create(hostType, dbType, path, sqlStatement, DbManagement::PERSISTENT, upgradeStatements);
    }

    void dbsync_teardown(void)
    {
        PipelineFactory::instance().release();
        DBSyncImplementation::instance().release();
    }

    TXN_HANDLE dbsync_create_txn(const DBSYNC_HANDLE handle,
                                 const cJSON* tables,
                                 const unsigned int threadNumber,
                                 const unsigned int maxQueueSize,
                                 callback_data_t callbackData)
    {
        std::string errorMessage;
        TXN_HANDLE txn {nullptr};

        if (!handle || !tables || !maxQueueSize || !callbackData.callback)
        {
            errorMessage += "Invalid parameters.";
        }
        else
        {
            try
            {
                const auto callbackWrapper {
                    [callbackData](ReturnTypeCallback result, const nlohmann::json& jsonResult)
                    {
                        const std::unique_ptr<cJSON, CJsonSmartDeleter> spJson {cJSON_Parse(jsonResult.dump().c_str())};
                        callbackData.callback(result, spJson.get(), callbackData.user_data);
                    }};
                const std::unique_ptr<char, CJsonSmartFree> spJsonBytes {cJSON_Print(tables)};
                txn = PipelineFactory::instance().create(
                    handle, nlohmann::json::parse(spJsonBytes.get()), threadNumber, maxQueueSize, callbackWrapper);
            }
            catch (const DbSync::dbsync_error& ex)
            {
                errorMessage += "DB error, id: " + std::to_string(ex.id()) + ". " + ex.what();
            }
            catch (...) // NOLINT(bugprone-empty-catch)
            {
                errorMessage += "Unrecognized error.";
            }
        }

        LogMessage(errorMessage);
        return txn;
    }

    int dbsync_close_txn(const TXN_HANDLE txn)
    {
        auto retVal {-1};
        std::string errorMessage;

        if (!txn)
        {
            errorMessage += "Invalid txn.";
        }
        else
        {
            try
            {
                PipelineFactory::instance().destroy(txn);
                retVal = 0;
            }
            catch (const DbSync::dbsync_error& ex)
            {
                errorMessage += "DB error, id: " + std::to_string(ex.id()) + ". " + ex.what();
                retVal = ex.id();
            }
            catch (...) // NOLINT(bugprone-empty-catch)
            {
                errorMessage += "Unrecognized error.";
            }
        }

        LogMessage(errorMessage);
        return retVal;
    }

    int dbsync_sync_txn_row(const TXN_HANDLE txn, const cJSON* jsInput)
    {
        auto retVal {-1};
        std::string error_message;

        if (!txn || !jsInput)
        {
            error_message += "Invalid txn or json.";
        }
        else
        {
            try
            {
                const std::unique_ptr<char, CJsonSmartFree> spJsonBytes {cJSON_PrintUnformatted(jsInput)};
                PipelineFactory::instance().pipeline(txn)->syncRow(nlohmann::json::parse(spJsonBytes.get()));
                retVal = 0;
            }
            catch (const DbSync::dbsync_error& ex)
            {
                error_message += "DB error, id: " + std::to_string(ex.id()) + ". " + ex.what();
                retVal = ex.id();
            }
            catch (...) // NOLINT(bugprone-empty-catch)
            {
                error_message += "Unrecognized error.";
            }
        }

        LogMessage(error_message);
        return retVal;
    }

    int dbsync_add_table_relationship(const DBSYNC_HANDLE handle, const cJSON* jsInput)
    {
        auto retVal {-1};
        std::string errorMessage;

        if (!handle || !jsInput)
        {
            errorMessage += "Invalid parameters.";
        }
        else
        {
            try
            {
                const std::unique_ptr<char, CJsonSmartFree> spJsonBytes {cJSON_Print(jsInput)};
                DBSyncImplementation::instance().addTableRelationship(handle, nlohmann::json::parse(spJsonBytes.get()));
                retVal = 0;
            }
            catch (const nlohmann::detail::exception& ex)
            {
                errorMessage += "json error, id: " + std::to_string(ex.id) + ". " + ex.what();
                retVal = ex.id;
            }
            catch (const DbSync::dbsync_error& ex)
            {
                errorMessage += "DB error, id: " + std::to_string(ex.id()) + ". " + ex.what();
                retVal = ex.id();
            }
            catch (...) // NOLINT(bugprone-empty-catch)
            {
                errorMessage += "Unrecognized error.";
            }
        }

        LogMessage(errorMessage);

        return retVal;
    }

    int dbsync_insert_data(const DBSYNC_HANDLE handle, const cJSON* jsInsert)
    {
        auto retVal {-1};
        std::string errorMessage;

        if (!handle || !jsInsert)
        {
            errorMessage += "Invalid handle or json.";
        }
        else
        {
            try
            {
                const std::unique_ptr<char, CJsonSmartFree> spJsonBytes {cJSON_Print(jsInsert)};
                DBSyncImplementation::instance().insertBulkData(handle, nlohmann::json::parse(spJsonBytes.get()));
                retVal = 0;
            }
            catch (const nlohmann::detail::exception& ex)
            {
                errorMessage += "json error, id: " + std::to_string(ex.id) + ". " + ex.what();
                retVal = ex.id;
            }
            catch (const DbSync::dbsync_error& ex)
            {
                errorMessage += "DB error, id: " + std::to_string(ex.id()) + ". " + ex.what();
                retVal = ex.id();
            }
            catch (const DbSync::max_rows_error& ex)
            {
                errorMessage += "DB error, ";
                errorMessage += ex.what();
            }
            catch (...) // NOLINT(bugprone-empty-catch)
            {
                errorMessage += "Unrecognized error.";
            }
        }

        LogMessage(errorMessage);

        return retVal;
    }

    int dbsync_set_table_max_rows(const DBSYNC_HANDLE handle, const char* table, const long long maxRows)
    {
        auto retVal {-1};
        std::string errorMessage;

        if (!handle || !table)
        {
            errorMessage += "Invalid parameters.";
        }
        else
        {
            try
            {
                DBSyncImplementation::instance().setMaxRows(handle, table, maxRows);
                retVal = 0;
            }
            catch (const DbSync::dbsync_error& ex)
            {
                errorMessage += "DB error, id: " + std::to_string(ex.id()) + ". " + ex.what();
                retVal = ex.id();
            }
            catch (...) // NOLINT(bugprone-empty-catch)
            {
                errorMessage += "Unrecognized error.";
            }
        }

        LogMessage(errorMessage);

        return retVal;
    }

    int dbsync_sync_row(const DBSYNC_HANDLE handle, const cJSON* jsInput, callback_data_t callbackData)
    {
        auto retVal {-1};
        std::string errorMessage;

        if (!handle || !jsInput || !callbackData.callback)
        {
            errorMessage += "Invalid input parameters.";
        }
        else
        {
            try
            {
                const auto callbackWrapper {
                    [callbackData](ReturnTypeCallback result, const nlohmann::json& jsonResult)
                    {
                        const std::unique_ptr<cJSON, CJsonSmartDeleter> spJson {cJSON_Parse(jsonResult.dump().c_str())};
                        callbackData.callback(result, spJson.get(), callbackData.user_data);
                    }};
                const std::unique_ptr<char, CJsonSmartFree> spJsonBytes {cJSON_PrintUnformatted(jsInput)};
                DBSyncImplementation::instance().syncRowData(
                    handle, nlohmann::json::parse(spJsonBytes.get()), callbackWrapper);
                retVal = 0;
            }
            catch (const nlohmann::detail::exception& ex)
            {
                errorMessage += "json error, id: " + std::to_string(ex.id) + ". " + ex.what();
                retVal = ex.id;
            }
            catch (const DbSync::dbsync_error& ex)
            {
                errorMessage += "DB error, id: " + std::to_string(ex.id()) + ". " + ex.what();
                retVal = ex.id();
            }
            catch (...) // NOLINT(bugprone-empty-catch)
            {
                errorMessage += "Unrecognized error.";
            }
        }

        LogMessage(errorMessage);
        return retVal;
    }

    int dbsync_select_rows(const DBSYNC_HANDLE handle, const cJSON* jsDataInput, callback_data_t callbackData)
    {
        auto retVal {-1};
        std::string errorMessage;

        if (!handle || !jsDataInput || !callbackData.callback)
        {
            errorMessage += "Invalid input parameters.";
        }
        else
        {
            try
            {
                const auto callbackWrapper {
                    [callbackData](ReturnTypeCallback result, const nlohmann::json& jsonResult)
                    {
                        const std::unique_ptr<cJSON, CJsonSmartDeleter> spJson {cJSON_Parse(jsonResult.dump().c_str())};
                        callbackData.callback(result, spJson.get(), callbackData.user_data);
                    }};
                const std::unique_ptr<char, CJsonSmartFree> spJsonBytes {cJSON_PrintUnformatted(jsDataInput)};
                DBSyncImplementation::instance().selectData(
                    handle, nlohmann::json::parse(spJsonBytes.get()), callbackWrapper);
                retVal = 0;
            }
            catch (const nlohmann::detail::exception& ex)
            {
                errorMessage += "json error, id: " + std::to_string(ex.id) + ". " + ex.what();
                retVal = ex.id;
            }
            catch (const DbSync::dbsync_error& ex)
            {
                errorMessage += "DB error, id: " + std::to_string(ex.id()) + ". " + ex.what();
                retVal = ex.id();
            }
            catch (...) // NOLINT(bugprone-empty-catch)
            {
                errorMessage += "Unrecognized error.";
            }
        }

        LogMessage(errorMessage);
        return retVal;
    }

    int dbsync_delete_rows(const DBSYNC_HANDLE handle, const cJSON* jsKeyValues)
    {
        auto retVal {-1};
        std::string errorMessage;

        if (!handle || !jsKeyValues)
        {
            errorMessage += "Invalid input parameters.";
        }
        else
        {
            try
            {
                const std::unique_ptr<char, CJsonSmartFree> spJsonBytes {cJSON_PrintUnformatted(jsKeyValues)};
                DBSyncImplementation::instance().deleteRowsData(handle, nlohmann::json::parse(spJsonBytes.get()));
                retVal = 0;
            }
            catch (const nlohmann::detail::exception& ex)
            {
                errorMessage += "json error, id: " + std::to_string(ex.id) + ". " + ex.what();
                retVal = ex.id;
            }
            catch (const DbSync::dbsync_error& ex)
            {
                errorMessage += "DB error, id: " + std::to_string(ex.id()) + ". " + ex.what();
                retVal = ex.id();
            }
            catch (...) // NOLINT(bugprone-empty-catch)
            {
                errorMessage += "Unrecognized error.";
            }
        }

        LogMessage(errorMessage);
        return retVal;
    }

    int dbsync_get_deleted_rows(const TXN_HANDLE txn, callback_data_t callbackData)
    {
        auto retVal {-1};
        std::string error_message;

        if (!txn || !callbackData.callback)
        {
            error_message += "Invalid txn or callback.";
        }
        else
        {
            try
            {
                const auto callbackWrapper {
                    [callbackData](ReturnTypeCallback result, const nlohmann::json& jsonResult)
                    {
                        const std::unique_ptr<cJSON, CJsonSmartDeleter> spJson {cJSON_Parse(jsonResult.dump().c_str())};
                        callbackData.callback(result, spJson.get(), callbackData.user_data);
                    }};
                PipelineFactory::instance().pipeline(txn)->getDeleted(callbackWrapper);
                retVal = 0;
            }
            catch (const DbSync::dbsync_error& ex)
            {
                error_message += "DB error, id: " + std::to_string(ex.id()) + ". " + ex.what();
                retVal = ex.id();
            }
            catch (...) // NOLINT(bugprone-empty-catch)
            {
                error_message += "Unrecognized error.";
            }
        }

        LogMessage(error_message);

        return retVal;
    }

    int dbsync_update_with_snapshot(const DBSYNC_HANDLE handle, const cJSON* jsSnapshot, cJSON** jsResult)
    {
        auto retVal {-1};
        std::string errorMessage;

        if (!handle || !jsSnapshot || !jsResult)
        {
            errorMessage += "Invalid input parameter.";
        }
        else
        {
            try
            {
                nlohmann::json result;
                const auto callbackWrapper {
                    [&result](ReturnTypeCallback resultType, const nlohmann::json& jsonResult)
                    {
                        static std::map<ReturnTypeCallback, std::string> s_opMap {
                            {MODIFIED, "modified"}, {DELETED, "deleted"}, {INSERTED, "inserted"}};
                        result[s_opMap.at(resultType)].push_back(jsonResult);
                    }};
                const std::unique_ptr<char, CJsonSmartFree> spJsonBytes {cJSON_PrintUnformatted(jsSnapshot)};
                DBSyncImplementation::instance().updateSnapshotData(
                    handle, nlohmann::json::parse(spJsonBytes.get()), callbackWrapper);
                *jsResult = cJSON_Parse(result.dump().c_str());
                retVal = 0;
            }
            catch (const nlohmann::detail::exception& ex)
            {
                errorMessage += "json error, id: " + std::to_string(ex.id) + ". " + ex.what();
                retVal = ex.id;
            }
            catch (const DbSync::dbsync_error& ex)
            {
                errorMessage += "DB error, id: " + std::to_string(ex.id()) + ". " + ex.what();
                retVal = ex.id();
            }
            catch (const DbSync::max_rows_error& ex)
            {
                errorMessage += "DB error, ";
                errorMessage += ex.what();
            }
            catch (...) // NOLINT(bugprone-empty-catch)
            {
                errorMessage += "Unrecognized error.";
            }
        }

        LogMessage(errorMessage);
        return retVal;
    }

    int
    dbsync_update_with_snapshot_cb(const DBSYNC_HANDLE handle, const cJSON* jsSnapshot, callback_data_t callbackData)
    {
        auto retVal {-1};
        std::string errorMessage;

        if (!handle || !jsSnapshot || !callbackData.callback)
        {
            errorMessage += "Invalid input parameters.";
        }
        else
        {
            try
            {
                const auto callbackWrapper {
                    [callbackData](ReturnTypeCallback result, const nlohmann::json& jsonResult)
                    {
                        const std::unique_ptr<cJSON, CJsonSmartDeleter> spJson {cJSON_Parse(jsonResult.dump().c_str())};
                        callbackData.callback(result, spJson.get(), callbackData.user_data);
                    }};
                const std::unique_ptr<char, CJsonSmartFree> spJsonBytes {cJSON_PrintUnformatted(jsSnapshot)};
                DBSyncImplementation::instance().updateSnapshotData(
                    handle, nlohmann::json::parse(spJsonBytes.get()), callbackWrapper);
                retVal = 0;
            }
            catch (const nlohmann::detail::exception& ex)
            {
                errorMessage += "json error, id: " + std::to_string(ex.id) + ". " + ex.what();
                retVal = ex.id;
            }
            catch (const DbSync::dbsync_error& ex)
            {
                errorMessage += "DB error, id: " + std::to_string(ex.id()) + ". " + ex.what();
                retVal = ex.id();
            }
            catch (...) // NOLINT(bugprone-empty-catch)
            {
                errorMessage += "Unrecognized error.";
            }
        }

        LogMessage(errorMessage);
        return retVal;
    }

    void dbsync_free_result(cJSON** jsData)
    {
        if (*jsData)
        {
            cJSON_Delete(*jsData);
        }
    }

#ifdef __cplusplus
}
#endif

void DBSync::initialize(std::function<void(const std::string&)> logFunction)
{
    if (!GS_LOG_FUNCTION)
    {
        GS_LOG_FUNCTION = std::move(logFunction);
    }
}

DBSync::DBSync(const HostType hostType,
               const DbEngineType dbType,
               const std::string& path,
               const std::string& sqlStatement,
               const DbManagement dbManagement,
               const std::vector<std::string>& upgradeStatements)
    : m_dbsyncHandle {DBSyncImplementation::instance().initialize(
          hostType, dbType, path, sqlStatement, dbManagement, upgradeStatements)}
    , m_shouldBeRemoved {true}
{
}

DBSync::DBSync(const DBSYNC_HANDLE dbsyncHandle)
    : m_dbsyncHandle {dbsyncHandle}
    , m_shouldBeRemoved {false}
{
}

DBSync::~DBSync()
{
    if (m_shouldBeRemoved)
    {
        DBSyncImplementation::instance().releaseContext(m_dbsyncHandle);
    }
}

void DBSync::teardown()
{
    PipelineFactory::instance().release();
    DBSyncImplementation::instance().release();
}

void DBSync::addTableRelationship(const nlohmann::json& jsInput)
{
    DBSyncImplementation::instance().addTableRelationship(m_dbsyncHandle, jsInput);
}

void DBSync::insertData(const nlohmann::json& jsInsert)
{
    DBSyncImplementation::instance().insertBulkData(m_dbsyncHandle, jsInsert);
}

void DBSync::setTableMaxRow(const std::string& table, const long long maxRows)
{
    DBSyncImplementation::instance().setMaxRows(m_dbsyncHandle, table, maxRows);
}

void DBSync::syncRow(const nlohmann::json& jsInput, ResultCallbackData& callbackData)
{
    const auto callbackWrapper {[callbackData](ReturnTypeCallback result, const nlohmann::json& jsonResult)
                                {
                                    callbackData(result, jsonResult);
                                }};
    DBSyncImplementation::instance().syncRowData(m_dbsyncHandle, jsInput, callbackWrapper);
}

void DBSync::selectRows(const nlohmann::json& jsInput, ResultCallbackData& callbackData)
{
    const auto callbackWrapper {[callbackData](ReturnTypeCallback result, const nlohmann::json& jsonResult)
                                {
                                    callbackData(result, jsonResult);
                                }};
    DBSyncImplementation::instance().selectData(m_dbsyncHandle, jsInput, callbackWrapper);
}

void DBSync::deleteRows(const nlohmann::json& jsInput)
{
    DBSyncImplementation::instance().deleteRowsData(m_dbsyncHandle, jsInput);
}

void DBSync::updateWithSnapshot(const nlohmann::json& jsInput, nlohmann::json& jsResult)
{
    const auto callbackWrapper {[&jsResult](ReturnTypeCallback resultType, const nlohmann::json& jsonResult)
                                {
                                    static std::map<ReturnTypeCallback, std::string> s_opMap {
                                        {MODIFIED, "modified"}, {DELETED, "deleted"}, {INSERTED, "inserted"}};
                                    jsResult[s_opMap.at(resultType)].push_back(jsonResult);
                                }};
    DBSyncImplementation::instance().updateSnapshotData(m_dbsyncHandle, jsInput, callbackWrapper);
}

void DBSync::updateWithSnapshot(const nlohmann::json& jsInput, ResultCallbackData& callbackData)
{
    const auto callbackWrapper {[callbackData](ReturnTypeCallback result, const nlohmann::json& jsonResult)
                                {
                                    callbackData(result, jsonResult);
                                }};
    DBSyncImplementation::instance().updateSnapshotData(m_dbsyncHandle, jsInput, callbackWrapper);
}

DBSyncTxn::DBSyncTxn(const DBSYNC_HANDLE handle,
                     const nlohmann::json& tables,
                     const unsigned int threadNumber,
                     const unsigned int maxQueueSize,
                     ResultCallbackData& callbackData)
    : m_shouldBeRemoved {true}
{
    const auto callbackWrapper {[callbackData](ReturnTypeCallback result, const nlohmann::json& jsonResult)
                                {
                                    callbackData(result, jsonResult);
                                }};
    m_txn = PipelineFactory::instance().create(handle, tables, threadNumber, maxQueueSize, callbackWrapper);
}

DBSyncTxn::DBSyncTxn(const TXN_HANDLE handle)
    : m_txn {handle}
    , m_shouldBeRemoved {false}
{
}

DBSyncTxn::~DBSyncTxn()
{
    if (m_shouldBeRemoved)
    {
        try
        {
            PipelineFactory::instance().destroy(m_txn);
        }
        catch (const DbSync::dbsync_error& ex)
        {
            LogMessage(ex.what());
        }
    }
}

void DBSyncTxn::syncTxnRow(const nlohmann::json& jsInput)
{
    PipelineFactory::instance().pipeline(m_txn)->syncRow(jsInput);
}

void DBSyncTxn::getDeletedRows(ResultCallbackData& callbackData)
{
    const auto callbackWrapper {[&callbackData](ReturnTypeCallback result, const nlohmann::json& jsonResult)
                                {
                                    callbackData(result, jsonResult);
                                }};
    PipelineFactory::instance().pipeline(m_txn)->getDeleted(callbackWrapper);
}

SelectQuery& SelectQuery::columnList(const std::vector<std::string>& fields)
{
    m_jsQuery["query"]["column_list"] = fields;
    return *this;
}

SelectQuery& SelectQuery::rowFilter(const std::string& filter)
{
    m_jsQuery["query"]["row_filter"] = filter;
    return *this;
}

SelectQuery& SelectQuery::distinctOpt(const bool distinct)
{
    m_jsQuery["query"]["distinct_opt"] = distinct;
    return *this;
}

SelectQuery& SelectQuery::orderByOpt(const std::string& orderBy)
{
    m_jsQuery["query"]["order_by_opt"] = orderBy;
    return *this;
}

SelectQuery& SelectQuery::countOpt(const uint32_t count)
{
    m_jsQuery["query"]["count_opt"] = count;
    return *this;
}

DeleteQuery& DeleteQuery::data(const nlohmann::json& data)
{
    m_jsQuery["query"]["data"].push_back(data);
    return *this;
}

DeleteQuery& DeleteQuery::reset()
{
    m_jsQuery["query"]["data"].clear();
    return *this;
}

DeleteQuery& DeleteQuery::rowFilter(const std::string& filter)
{
    m_jsQuery["query"]["where_filter_opt"] = filter;
    return *this;
}

InsertQuery& InsertQuery::data(const nlohmann::json& data)
{
    m_jsQuery["data"].push_back(data);
    return *this;
}

InsertQuery& InsertQuery::reset()
{
    m_jsQuery["data"].clear();
    return *this;
}

SyncRowQuery& SyncRowQuery::data(const nlohmann::json& data)
{
    m_jsQuery["data"].push_back(data);
    return *this;
}

SyncRowQuery& SyncRowQuery::ignoreColumn(const std::string& column)
{
    m_jsQuery["options"]["ignore"].push_back(column);
    return *this;
}

SyncRowQuery& SyncRowQuery::returnOldData()
{
    m_jsQuery["options"]["return_old_data"] = true;
    return *this;
}

SyncRowQuery& SyncRowQuery::reset()
{
    m_jsQuery["data"].clear();
    return *this;
}
