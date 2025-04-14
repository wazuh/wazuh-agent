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
    /// @brief Type alias for a function that loads a policy from a SCA Policy yaml file
    using PolicyLoaderFunc = std::function<SCAPolicy(const std::filesystem::path&)>;

    /// @brief Constructor for SCAPolicyLoader
    /// @param fileSystemWrapper A shared pointer to a file system wrapper
    /// @param configurationParser A shared pointer to a configuration parser
    /// @param dBSync A shared pointer to a DBSync object
    /// @param loader A function that loads a policy from a SCA Policy yaml file
    SCAPolicyLoader(std::shared_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr,
                    std::shared_ptr<const configuration::ConfigurationParser> configurationParser = nullptr,
                    std::shared_ptr<IDBSync> dBSync = nullptr,
                    PolicyLoaderFunc loader = SCAPolicy::LoadFromFile);

    /// @brief Destructor for SCAPolicyLoader
    ~SCAPolicyLoader() = default;

    /// @brief Loads SCA Policies
    /// @returns a vector of SCAPolicy objects
    std::vector<SCAPolicy> GetPolicies() const;

    /// @brief Saves SCA Policies into the database
    /// @param data All SCA policies and its checks
    /// @param createEvents Callback function to generate events. It will be called with two
    /// maps:
    ///   - modifiedPoliciesMap: maps policy ID to the JSON data of the created, modified or deleted policy
    ///   - modifiedChecksMap: maps check ID to the JSON data of the created, modified or deleted check
    void SyncPoliciesAndReportDelta(const nlohmann::json& data, const CreateEventsFunc& createEvents);

private:
    std::unordered_map<std::string, nlohmann::json> SyncWithDBSync(const nlohmann::json& data,
                                                                   const std::string& tableName);
    void UpdateCheckResult(const nlohmann::json& check);

    std::shared_ptr<IFileSystemWrapper> m_fileSystemWrapper;
    PolicyLoaderFunc m_policyLoader;

    std::vector<std::filesystem::path> m_customPoliciesPaths;
    std::vector<std::filesystem::path> m_disabledPoliciesPaths;

    std::shared_ptr<IDBSync> m_dBSync;
};
