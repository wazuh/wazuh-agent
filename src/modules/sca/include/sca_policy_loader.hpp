#pragma once

#include <isca_policy_loader.hpp>

#include <configuration_parser.hpp>
#include <idbsync.hpp>
#include <ifilesystem_wrapper.hpp>
#include <message.hpp>
#include <sca_policy.hpp>

#include <filesystem>
#include <functional>
#include <memory>
#include <nlohmann/json.hpp>
#include <string>
#include <vector>

/// @brief Default DBSync queue size
constexpr auto DBSYNC_QUEUE_SIZE {4096};

/// @brief Default DBSync table names
constexpr auto SCA_POLICY_TABLE_NAME {"sca_policy"};
constexpr auto SCA_CHECK_TABLE_NAME {"sca_check"};

/// @brief Type alias for a function that creates events
using CreateEventsFunc = std::function<void(std::unordered_map<std::string, nlohmann::json> modifiedPoliciesMap,
                                            std::unordered_map<std::string, nlohmann::json> modifiedChecksMap)>;

class SCAPolicyLoader : public ISCAPolicyLoader
{
public:
    /// @brief Constructor for SCAPolicyLoader
    /// @param fileSystemWrapper A shared pointer to a file system wrapper
    /// @param configurationParser A shared pointer to a configuration parser
    /// @param dBSync A shared pointer to a DBSync object
    SCAPolicyLoader(std::shared_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr,
                    std::shared_ptr<const configuration::ConfigurationParser> configurationParser = nullptr,
                    std::shared_ptr<IDBSync> dBSync = nullptr);

    /// @brief Destructor for SCAPolicyLoader
    ~SCAPolicyLoader() = default;

    /// @brief Loads SCA Policies
    /// @returns a vector of SCAPolicy objects
    /// @param createEvents Callback function to generate events. It will be called with two
    /// maps:
    ///   - modifiedPoliciesMap: maps policy ID to the JSON data of the created, modified or deleted policy
    ///   - modifiedChecksMap: maps check ID to the JSON data of the created, modified or deleted check
    std::vector<SCAPolicy> LoadPolicies(const CreateEventsFunc& createEvents) const;

    /// @brief Saves SCA Policies into the database
    /// @param data All SCA policies and its checks
    /// @param createEvents Callback function to generate events. It will be called with two
    /// maps:
    ///   - modifiedPoliciesMap: maps policy ID to the JSON data of the created, modified or deleted policy
    ///   - modifiedChecksMap: maps check ID to the JSON data of the created, modified or deleted check
    void SyncPoliciesAndReportDelta(const nlohmann::json& data, const CreateEventsFunc& createEvents) const;

private:
    /// @brief Synchronizes with the DBSync and returns a map of modified policies and checks
    /// @param data SCA policies and checks
    /// @param tableName DBSync table name
    /// @returns a map of modified policies and checks
    std::unordered_map<std::string, nlohmann::json> SyncWithDBSync(const nlohmann::json& data,
                                                                   const std::string& tableName) const;

    /// @brief Synchronizes with the DBSync and updates the result of a check
    /// @param check The check to update
    void UpdateCheckResult(const nlohmann::json& check) const;

    std::shared_ptr<IFileSystemWrapper> m_fileSystemWrapper;

    std::vector<std::filesystem::path> m_customPoliciesPaths;
    std::vector<std::filesystem::path> m_disabledPoliciesPaths;

    std::shared_ptr<IDBSync> m_dBSync;
};
