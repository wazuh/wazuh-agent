#pragma once

#include <cjson/cJSON.h>

/// @brief Represents the different host types to be used.
typedef enum
{
    MANAGER = 0,
    AGENT = 1
} HostType;

/// @brief Represents the database type to be used.
typedef enum
{
    UNDEFINED = 0, /*< Undefined database. */
    SQLITE3 = 1,   /*< SQLite3 database.   */
} DbEngineType;

/// @brief Configures how the engine manages the database at startup.
typedef enum
{
    VOLATILE = 0,   /*< Removes the DB every time .                          */
    PERSISTENT = 1, /*< The DB is kept and the correct version is checked.   */
} DbManagement;

/// @brief Represents the database operation events.
typedef enum
{
    MODIFIED = 0, /*< Database modificaton operation.         */
    DELETED = 1,  /*< Database deletion operation.            */
    INSERTED = 2, /*< Database insertion operation.           */
    MAX_ROWS = 3, /*< Database has reached max rows number.   */
    DB_ERROR = 4, /*< Internal failure.                       */
    SELECTED = 5, /*< Database select operation.              */
    GENERIC = 6   /*< Generic result for reuse.               */
} ReturnTypeCallback;

/// @brief Represents the handle associated with database creation.
typedef const void* DBSYNC_HANDLE;

/// @brief Represents the transaction handle associated with a database instance.
typedef const void* TXN_HANDLE;

/// @brief Callback function for results
/// @param result_type Enumeration value indicating what action was taken.
/// @param result_json Json which describe the change.
/// @param user_data   User data space returned.
/// @details Callback called for each obtained result, after evaluating changes between two snapshots.
typedef void (*result_callback_t)(ReturnTypeCallback result_type, const cJSON* result_json, void* user_data);

/// @struct callback_data_t
/// This struct contains the result callback will be called for each result
/// and user data space returned in each callback call.
/// The instance of this structure lives in the library's consumer ecosystem.
typedef struct
{
    result_callback_t callback; /*< Result callback. */
    void* user_data;            /*< User data space returned in each callback. */
} callback_data_t;

/// @brief Callback function for user defined logging.
/// @param msg Message to be logged.
/// @details Useful to get deeper information during the dbsync interaction.
typedef void (*log_fnc_t)(const char* msg);

/// @brief Definition to indicate the unlimited queue.
/// @details It's used to define the unlimited queue size.
#define UNLIMITED_QUEUE_SIZE 0
