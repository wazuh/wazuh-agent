#pragma once

#include <dbsync.hpp>
#include <idbsync.hpp>
#include <logger.hpp>

#include <nlohmann/json.hpp>

/// @brief Class for handling SCA events
class SCAEventHandler
{
public:
    /// @brief Constructor
    /// @param dBSync A shared pointer to a DBSync object
    SCAEventHandler(std::shared_ptr<IDBSync> dBSync = nullptr);

    /// @brief Creates a json array of events
    /// @param modifiedPoliciesMap Map of modified policies
    /// @param modifiedChecksMap Map of modified checks. During the execution of this function, checks that have already
    /// been reported due to changes in a policy will be removed from this map to avoid sending them twice or iterating
    /// over them unnecessarily.
    void CreateEvents(const std::unordered_map<std::string, nlohmann::json>& modifiedPoliciesMap,
                      const std::unordered_map<std::string, nlohmann::json>& modifiedChecksMap) const;

private:
    std::vector<nlohmann::json> GetChecksForPolicy(const std::string& policyId) const;
    nlohmann::json GetPolicyById(const std::string& policyId) const;

    std::shared_ptr<IDBSync> m_dBSync;
};
