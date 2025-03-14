#pragma once

#include "commonDefs.h"
#include "dbengine_factory.h"
#include <map>
#include <memory>
#include <mutex>
#include <nlohmann/json.hpp>
#include <shared_mutex>

namespace DbSync
{
    /// @brief DBSync implementation
    class DBSyncImplementation final
    {
    public:
        /// @brief Singleton
        static DBSyncImplementation& instance()
        {
            static DBSyncImplementation s_instance;
            return s_instance;
        }

        /// @brief Insert the \p json data in the database.
        /// @param handle Handle assigned as part of the \ref dbsync_create method().
        /// @param json   JSON information with values to be inserted.
        void insertBulkData(const DBSYNC_HANDLE handle, const nlohmann::json& json);

        /// @brief Synchronize the row data in the database.
        /// @param handle Handle assigned as part of the \ref dbsync_create method().
        /// @param json   JSON information with values to be inserted.
        /// @param callback callback
        void syncRowData(const DBSYNC_HANDLE handle, const nlohmann::json& json, const ResultCallback callback);

        /// @brief Synchronize the row data in the database.
        /// @param handle Handle assigned as part of the \ref dbsync_create method().
        /// @param txnHandle Database transaction to be used.
        /// @param json   JSON information with values to be inserted.
        /// @param callback callback
        void syncRowData(const DBSYNC_HANDLE handle,
                         const TXN_HANDLE txnHandle,
                         const nlohmann::json& json,
                         const ResultCallback callback);

        /// @brief Delete the row data in the database.
        /// @param handle Handle assigned as part of the \ref dbsync_create method().
        /// @param json   JSON information with values to be inserted.
        void deleteRowsData(const DBSYNC_HANDLE handle, const nlohmann::json& json);

        /// @brief Update the snapshot data in the database.
        /// @param handle Handle assigned as part of the \ref dbsync_create method().
        /// @param json   JSON information with values to be inserted.
        /// @param callback callback
        void updateSnapshotData(const DBSYNC_HANDLE handle, const nlohmann::json& json, const ResultCallback callback);

        /// @brief Initialize DBSync.
        /// @param hostType Dynamic library host type to be used.
        /// @param dbType Database type to be used (currently only supported SQLITE3)
        /// @param path Path where the local database will be created.
        /// @param sqlStatement SQL sentence to create tables in a SQL engine.
        /// @param dbManagement Database management type to be used at startup.
        /// @param upgradeStatements SQL sentences to be executed when upgrading the database.
        /// @return Handle instance to be used for common sql operations (cannot be used by more than 1 thread).
        DBSYNC_HANDLE initialize(const HostType hostType,
                                 const DbEngineType dbType,
                                 const std::string& path,
                                 const std::string& sqlStatement,
                                 const DbManagement dbManagement,
                                 const std::vector<std::string>& upgradeStatements);

        /// @brief Sets the maximum number of rows for a table
        /// @param handle Handle assigned as part of the \ref dbsync_create method().
        /// @param table Table name to apply the max rows configuration.
        /// @param maxRows Max rows number to be applied in the table \p table table.
        void setMaxRows(const DBSYNC_HANDLE handle, const std::string& table, const long long maxRows);

        /// @brief Creates a new transaction.
        /// @param handle Handle assigned as part of the \ref dbsync_create method().
        /// @param json JSON information with values to be inserted.
        /// @return Handle instance to be used for common sql operations (cannot be used by more than 1 thread).
        TXN_HANDLE createTransaction(const DBSYNC_HANDLE handle, const nlohmann::json& json);

        /// @brief Closes a transaction.
        /// @param handle Handle assigned as part of the \ref dbsync_create method().
        /// @param txnHandle Database transaction to be closed.
        void closeTransaction(const DBSYNC_HANDLE handle, const TXN_HANDLE txnHandle);

        /// @brief Gets the deleted rows (diff) from the database.
        /// @param handle Handle assigned as part of the \ref dbsync_create method().
        /// @param txnHandle Database transaction to be used.
        /// @param callback callback
        void getDeleted(const DBSYNC_HANDLE handle, const TXN_HANDLE txnHandle, const ResultCallback callback);

        /// @brief Select data from the database.
        /// @param handle Handle assigned as part of the \ref dbsync_create method().
        /// @param json JSON information with values to be inserted.
        /// @param callback callback
        void selectData(const DBSYNC_HANDLE handle, const nlohmann::json& json, const ResultCallback& callback);

        /// @brief Add a table relationship.
        /// @param handle Handle assigned as part of the \ref dbsync_create method().
        /// @param json JSON information with values to be inserted.
        void addTableRelationship(const DBSYNC_HANDLE handle, const nlohmann::json& json);

        /// @brief Release the DBSync instance.
        void release();

        /// @brief Release the DBSync context.
        /// @param handle Handle assigned as part of the \ref dbsync_create method().
        void releaseContext(const DBSYNC_HANDLE handle);

    private:
        /// @brief Transaction context
        struct TransactionContext final
        {
            /// @brief Transaction context constructor
            explicit TransactionContext(const nlohmann::json& tables)
                : m_tables(std::move(tables))
            {
            }

            nlohmann::json m_tables;
        };

        /// @brief DB engine context
        class DbEngineContext final
        {
        public:
            /// @brief DB engine context constructor
            DbEngineContext(std::unique_ptr<IDbEngine>& dbEngine, const HostType hostType, const DbEngineType dbType)
                : m_dbEngine {std::move(dbEngine)}
                , m_hostType {hostType}
                , m_dbEngineType {dbType}
            {
            }

            const std::unique_ptr<IDbEngine> m_dbEngine;
            const HostType m_hostType;
            const DbEngineType m_dbEngineType;

            /// @brief Transaction context getter
            /// @param handle Handle assigned as part of the \ref dbsync_create method().
            /// @return Transaction context
            const std::shared_ptr<DBSyncImplementation::TransactionContext> transactionContext(const TXN_HANDLE handle)
            {
                std::lock_guard<std::mutex> lock {m_mutex};
                const auto it {m_transactionContexts.find(handle)};

                if (m_transactionContexts.end() == it)
                {
                    throw dbsync_error {INVALID_TRANSACTION};
                }

                return it->second;
            }

            /// @brief Transaction context setter
            /// @param spTransactionContext Transaction context.
            void addTransactionContext(
                const std::shared_ptr<DbSync::DBSyncImplementation::TransactionContext>& spTransactionContext)
            {
                std::lock_guard<std::mutex> lock {m_mutex};
                m_transactionContexts[spTransactionContext.get()] = spTransactionContext;
            }

            /// @brief Transaction context deleter
            /// @param txnHandle Handle assigned as part of the \ref dbsync_create method().
            void deleteTransactionContext(const TXN_HANDLE txnHandle)
            {
                std::lock_guard<std::mutex> lock {m_mutex};
                m_transactionContexts.erase(txnHandle);
            }

            std::shared_timed_mutex m_syncMutex;

        private:
            std::map<TXN_HANDLE, std::shared_ptr<TransactionContext>> m_transactionContexts;
            std::mutex m_mutex;
        };

        /// @brief DB engine context getter
        /// @param handle Handle assigned as part of the \ref dbsync_create method().
        /// @return DB engine context
        std::shared_ptr<DbEngineContext> dbEngineContext(const DBSYNC_HANDLE handle);

        /// @brief Default DBsync implementation constructor
        DBSyncImplementation() = default;

        /// @brief Default DBsync implementation destructor
        ~DBSyncImplementation() = default;

        /// @brief Delete DBsync implementation copy constructor
        DBSyncImplementation(const DBSyncImplementation&) = delete;

        /// @brief Delete DBsync implementation copy assignment
        DBSyncImplementation& operator=(const DBSyncImplementation&) = delete;

        std::map<DBSYNC_HANDLE, std::shared_ptr<DbEngineContext>> m_dbSyncContexts;
        std::mutex m_mutex;
    };
} // namespace DbSync
