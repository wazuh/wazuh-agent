#pragma once

#include <idbsync.hpp>
#include <message.hpp>

#include <nlohmann/json.hpp>

/// @brief Class for handling SCA events
class SCAEventHandler
{
public:
    /// @brief Constructor
    /// @param agentUUID The UUID of the agent
    /// @param dBSync A shared pointer to a DBSync object
    /// @param pushMessage A function that push a message into the queue
    SCAEventHandler(std::string agentUUID,
                    std::shared_ptr<IDBSync> dBSync = nullptr,
                    std::function<int(Message)> pushMessage = nullptr);

    /// @brief Creates a json array of events
    /// @param modifiedPoliciesMap Map of modified policies
    /// @param modifiedChecksMap Map of modified checks. During the execution of this function, checks that have already
    /// been reported due to changes in a policy will be removed from this map to avoid sending them twice or iterating
    /// over them unnecessarily.
    void CreateEvents(const std::unordered_map<std::string, nlohmann::json>& modifiedPoliciesMap,
                      const std::unordered_map<std::string, nlohmann::json>& modifiedChecksMap) const;

private:
    void ProcessStateful(const nlohmann::json& event) const;
    std::string CalculateHashId(const nlohmann::json& data) const;
    void PushMessage(const nlohmann::json& event, const nlohmann::json& metadata) const;

    std::vector<nlohmann::json> GetChecksForPolicy(const std::string& policyId) const;
    nlohmann::json GetPolicyById(const std::string& policyId) const;
    nlohmann::json StringToJsonArray(const std::string& input) const;

    std::string m_agentUUID;
    std::shared_ptr<IDBSync> m_dBSync;
    std::function<int(Message)> m_pushMessage;
};
