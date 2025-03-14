#pragma once

#include "commonDefs.h"

#ifdef __cplusplus
extern "C"
{
#endif

    /// @brief Initializes the shared library.
    /// @param logFunction pointer to log function to be used by the dbsync.
    void dbsync_initialize(log_fnc_t logFunction);

    /// @brief Creates a new DBSync instance (wrapper)
    /// @param hostType          Dynamic library host type to be used.
    /// @param dbType            Database type to be used (currently only supported SQLITE3)
    /// @param path              Path where the local database will be created.
    /// @param sqlStatement      SQL sentence to create tables in a SQL engine.
    /// @param dbManagement      Database management type to be used at startup.
    /// @param upgradeStatements SQL sentences to upgrade tables in a SQL engine.
    /// @return Handle instance to be used for common sql operations (cannot be used by more than 1 thread).
    DBSYNC_HANDLE _dbsync_create(const HostType hostType,
                                 const DbEngineType dbType,
                                 const char* path,
                                 const char* sqlStatement,
                                 const DbManagement dbManagement,
                                 const char** upgradeStatements);

    /// @brief Creates a new DBSync instance (wrapper)
    /// @param hostType          Dynamic library host type to be used.
    /// @param dbType            Database type to be used (currently only supported SQLITE3)
    /// @param path              Path where the local database will be created.
    /// @param sqlStatement      SQL sentence to create tables in a SQL engine.
    /// @note dbManagement will be DbManagement::VOLATILE as default and upgradeStatements will be NULL.
    /// @return Handle instance to be used for common sql operations (cannot be used by more than 1 thread).
    DBSYNC_HANDLE
    dbsync_create(const HostType hostType, const DbEngineType dbType, const char* path, const char* sqlStatement);

    /// @brief Creates a new DBSync instance (wrapper)
    /// @param hostType          Dynamic library host type to be used.
    /// @param dbType            Database type to be used (currently only supported SQLITE3)
    /// @param path              Path where the local database will be created.
    /// @param sqlStatement      SQL sentence to create tables in a SQL engine.
    /// @param upgradeStatements SQL sentences to upgrade tables in a SQL engine.
    /// @note dbManagement will be DbManagement::PERSISTENT as default.
    /// @return Handle instance to be used for common sql operations (cannot be used by more than 1 thread).
    DBSYNC_HANDLE dbsync_create_persistent(const HostType hostType,
                                           const DbEngineType dbType,
                                           const char* path,
                                           const char* sqlStatement,
                                           const char** upgradeStatements);

    /// @brief Turns off the services provided by the shared library.
    void dbsync_teardown(void);

    /// @brief Creates a database transaction based on the supplied information.
    /// @param handle         Handle obtained from the \ref dbsync_create method call.
    /// @param tables         Tables to be created in the transaction.
    /// @param threadNumber   Number of worker threads for processing data. If 0 hardware concurrency
    ///                       value will be used.
    /// @param maxQueueSize   Max data number to hold/queue to be processed.
    /// @param callbackData   This struct contain the result callback will be called for each result
    ///                       and user data space returned in each callback call.
    /// @return Handle instance to be used in transacted operations.
    TXN_HANDLE dbsync_create_txn(const DBSYNC_HANDLE handle,
                                 const cJSON* tables,
                                 const unsigned int threadNumber,
                                 const unsigned int maxQueueSize,
                                 callback_data_t callbackData);

    /// @brief Closes the \p txn database transaction.
    /// @param txn     Database transaction to be closed.
    /// @return 0 if succeeded,
    ///         specific error code (OS dependent) otherwise.
    int dbsync_close_txn(const TXN_HANDLE txn);

    /// @brief Synchronizes the \p jsInput data using the \p txn current
    ///  database transaction.
    /// @param txn      Database transaction to be used for \p jsInput data sync.
    /// @param jsInput  JSON information to be synchronized.
    /// @return 0 if succeeded,
    ///         specific error code (OS dependent) otherwise.
    int dbsync_sync_txn_row(const TXN_HANDLE txn, const cJSON* jsInput);

    /// @brief Generates triggers that execute actions to maintain consistency between tables.
    /// @param handle        Handle assigned as part of the \ref dbsync_create method.
    /// @param jsInput       JSON information with tables relationship.
    /// @return 0 if succeeded,
    ///         specific error code (OS dependent) otherwise.
    int dbsync_add_table_relationship(const DBSYNC_HANDLE handle, const cJSON* jsInput);

    /// @brief Insert the \p jsInsert data in the database.
    /// @param handle    Handle assigned as part of the \ref dbsync_create method().
    /// @param jsInsert  JSON information with values to be inserted.
    /// @return 0 if succeeded,
    ///         specific error code (OS dependent) otherwise.
    int dbsync_insert_data(const DBSYNC_HANDLE handle, const cJSON* jsInsert);

    /// @brief Sets the max rows in the \p table table.
    /// @param handle   Handle assigned as part of the \ref dbsync_create method().
    /// @param table    Table name to apply the max rows configuration.
    /// @param maxRows  Max rows number to be applied in the table \p table table.
    /// @return 0 if succeeded,
    ///         specific error code (OS dependent) otherwise.
    int dbsync_set_table_max_rows(const DBSYNC_HANDLE handle, const char* table, const long long maxRows);

    /// @brief Inserts (or modifies) a database record.
    /// @param handle         Handle instance assigned as part of the \ref dbsync_create method().
    /// @param jsInput        JSON information used to add/modified a database record.
    /// @param callbackData   This struct contains the result callback that will be called for each result
    ///                       and user data space returned in each callback call.
    /// @return 0 if succeeded,
    ///         specific error code (OS dependent) otherwise.
    int dbsync_sync_row(const DBSYNC_HANDLE handle, const cJSON* jsInput, callback_data_t callbackData);

    /// @brief Select data, based in \p json_data_input data, from the database table.
    /// @param handle          Handle assigned as part of the \ref dbsync_create method().
    /// @param jsDataInput     JSON with table name, fields, filters and options to apply in the query.
    /// @param callbackData    This struct contain the result callback will be called for each result
    ///                        and user data space returned in each callback call.
    /// @return 0 if succeeded,
    ///         specific error code (OS dependent) otherwise.
    int dbsync_select_rows(const DBSYNC_HANDLE handle, const cJSON* jsDataInput, callback_data_t callbackData);

    /// @brief Deletes a database table record and its relationships based on \p jsKeyValues value.
    /// @param handle        Handle instance assigned as part of the \ref dbsync_create method().
    /// @param jsKeyValues   JSON information to be applied/deleted in the database.
    /// @return 0 if succeeded,
    ///         specific error code (OS dependent) otherwise.
    int dbsync_delete_rows(const DBSYNC_HANDLE handle, const cJSON* jsKeyValues);

    /// @brief Gets the deleted rows (diff) from the database.
    /// @param txn             Database transaction to be used.
    /// @param callbackData    This struct contain the result callback will be called for each result
    ///                        and user data space returned in each callback call.
    /// @return 0 if succeeded,
    ///         specific error code (OS dependent) otherwise.
    int dbsync_get_deleted_rows(const TXN_HANDLE txn, callback_data_t callbackData);

    /// @brief Updates data table with \p jsSnapshot information. \p jsResult value will
    ///  hold/contain the results of this operation (rows insertion, modification and/or deletion).
    /// @param handle      Handle instance assigned as part of the \ref dbsync_create method().
    /// @param jsSnapshot  JSON information with snapshot values.
    /// @param jsResult    JSON with deletes, creations and modifications (diffs) in rows.
    /// @return 0 if succeeded,
    ///         specific error code (OS dependent) otherwise.
    /// @details The \p jsResult resulting data should be freed using the \ref dbsync_free_result function.
    int dbsync_update_with_snapshot(const DBSYNC_HANDLE handle, const cJSON* jsSnapshot, cJSON** jsResult);

    /// @brief Update data table, based on json_raw_snapshot bulk data based on json string.
    /// @param handle          Handle assigned as part of the \ref dbsync_create method().
    /// @param jsSnapshot      JSON with snapshot values.
    /// @param callbackData    This struct contain the result callback will be called for each result
    ///                        and user data space returned in each callback call.
    /// @return 0 if succeeded,
    ///         specific error code (OS dependent) otherwise.
    int
    dbsync_update_with_snapshot_cb(const DBSYNC_HANDLE handle, const cJSON* jsSnapshot, callback_data_t callbackData);

    /// @brief Deallocate cJSON result data.
    /// @param jsData JSON information be be deallocated.
    /// @details This function should only be used to free result objects obtained
    ///  from the interface.
    void dbsync_free_result(cJSON** jsData);

#ifdef __cplusplus
}
#endif
