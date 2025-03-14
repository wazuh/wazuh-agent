#pragma once

#include "abstractLocking.hpp"
#include "commonDefs.h"
#include <functional>
#include <nlohmann/json.hpp>
#include <set>
#include <shared_mutex>
#include <string>
#include <vector>

namespace DbSync
{
    using ResultCallback = std::function<void(ReturnTypeCallback, const nlohmann::json&)>;

    /// @brief Interface for DbEngines
    class IDbEngine
    {
    public:
        /// @brief Default destructor
        virtual ~IDbEngine() = default;

        /// @brief Inserts data into a table
        /// @param table table name
        /// @param data data to be inserted
        virtual void bulkInsert(const std::string& table, const nlohmann::json& data) = 0;

        /// @brief Refreshes table data
        /// @param data data
        /// @param callback callback
        /// @param lock lock
        virtual void refreshTableData(const nlohmann::json& data,
                                      const ResultCallback callback,
                                      std::unique_lock<std::shared_timed_mutex>& lock) = 0;

        /// @brief Syncs table row data
        /// @param jsInput JSON input
        /// @param callback callback
        /// @param inTransaction transaction
        /// @param mutex mutex
        virtual void syncTableRowData(const nlohmann::json& jsInput,
                                      const ResultCallback callback,
                                      const bool inTransaction,
                                      ILocking& mutex) = 0;

        /// @brief Sets the maximum number of rows for a table
        /// @param table table name
        /// @param maxRows maximum number of rows
        virtual void setMaxRows(const std::string& table, const int64_t maxRows) = 0;

        /// @brief Sets the status field for a table
        /// @param tableNames table names
        virtual void initializeStatusField(const nlohmann::json& tableNames) = 0;

        /// @brief Deletes rows by status field
        /// @param tableNames table names
        virtual void deleteRowsByStatusField(const nlohmann::json& tableNames) = 0;

        /// @brief Returns rows marked for delete
        /// @param tableNames table names
        /// @param callback callback
        /// @param lock lock
        virtual void returnRowsMarkedForDelete(const nlohmann::json& tableNames,
                                               const DbSync::ResultCallback callback,
                                               std::unique_lock<std::shared_timed_mutex>& lock) = 0;

        /// @brief Selects data from a table
        /// @param table table name
        /// @param query query
        /// @param callback callback
        /// @param lock lock
        virtual void selectData(const std::string& table,
                                const nlohmann::json& query,
                                const ResultCallback& callback,
                                std::unique_lock<std::shared_timed_mutex>& lock) = 0;

        /// @brief Deletes table rows data
        /// @param table table name
        /// @param jsDeletionData JSON deletion data
        virtual void deleteTableRowsData(const std::string& table, const nlohmann::json& jsDeletionData) = 0;

        /// @brief Adds table relationship
        /// @param data JSON data
        virtual void addTableRelationship(const nlohmann::json& data) = 0;

    protected:
        /// @brief Default constructor
        IDbEngine() = default;
    };
} // namespace DbSync
