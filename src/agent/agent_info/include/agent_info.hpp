#pragma once

#include <nlohmann/json.hpp>

#include <string>
#include <vector>

/// @brief Stores and manages information about an agent.
///
/// This class provides methods for getting and setting the agent's key,
/// UUID, and groups. It also includes private methods for creating and
/// validating the key.
class AgentInfo
{
public:
    /// @brief Default constructor for the AgentInfo class.
    AgentInfo();

    /// @brief Gets the agent's key.
    /// @return The agent's key.
    std::string GetKey() const;

    /// @brief Gets the agent's UUID.
    /// @return The agent's UUID.
    std::string GetUUID() const;

    /// @brief Gets the agent's groups.
    /// @return A vector of the agent's groups.
    std::vector<std::string> GetGroups() const;

    /// @brief Sets the agent's key.
    /// @param key The agent's new key.
    /// @return True if the key was successfully set, false otherwise.
    bool SetKey(const std::string& key);

    /// @brief Sets the agent's UUID.
    /// @param uuid The agent's new UUID.
    void SetUUID(const std::string& uuid);

    /// @brief Sets the agent's groups.
    /// @param groupList A vector of the agent's new groups.
    void SetGroups(const std::vector<std::string>& groupList);

    /// @brief Gets the agent's type.
    /// @return The agent's type.
    std::string GetType() const;

    /// @brief Gets the agent's version.
    /// @return The agent's version.
    std::string GetVersion() const;

    /// @brief Gets information about OS and networks.
    /// @return A json object with information about OS and networks.
    nlohmann::json GetEndpointInfo() const;

    /// @brief Gets all the information about the agent.
    /// @param includeKey Indicates whether or not the Key should be included in the answer..
    /// @return A json object with all information about the agent.
    nlohmann::json GetMetadataInfo(const bool includeKey) const;

private:
    /// @brief Creates a random key for the agent.
    ///
    /// The key is 32 alphanumeric characters.
    ///
    /// @return A randomly generated key.
    std::string CreateKey() const;

    /// @brief Validates a given key.
    /// @param key The key to validate.
    /// @return True if the key is valid (32 alphanumeric characters), false
    /// otherwise.
    bool ValidateKey(const std::string& key) const;

    /// @brief The agent's key.
    std::string m_key;

    /// @brief The agent's UUID.
    std::string m_uuid;

    /// @brief The agent's groups.
    std::vector<std::string> m_groups;
};
