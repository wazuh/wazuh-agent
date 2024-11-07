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
    /// @brief Constructs an AgentInfo object with OS and network information retrieval functions.
    ///
    /// This constructor initializes the AgentInfo object by setting up OS and network
    /// information retrieval functions. It also generates a UUID for the agent if one
    /// does not already exist, and loads endpoint, metadata, and header information.
    ///
    /// @param getOSInfo Function to retrieve OS information in JSON format.
    /// @param getNetworksInfo Function to retrieve network information in JSON format.
    AgentInfo(const std::function<nlohmann::json()>& getOSInfo,
              const std::function<nlohmann::json()>& getNetworksInfo);

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

    /// @brief Gets the agent information for the request header.
    /// @return A string with the information for the request header.
    std::string GetHeaderInfo() const;

    /// @brief Gets all the information about the agent.
    /// @param agentIsRegistering Indicates if the agent is about to register.
    /// @return A json object with all information about the agent.
    nlohmann::json GetMetadataInfo(const bool agentIsRegistering) const;

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

    /// @brief Loads the endpoint information into `m_endpointInfo`.
    void LoadEndpointInfo();

    /// @brief Loads the metadata information into `m_metadataInfo`.
    void LoadMetadataInfo();

    /// @brief Loads the header information into `m_headerInfo`.
    void LoadHeaderInfo();

    /// @brief Extracts the active IP address from the network JSON data.
    /// @param networksJson JSON object containing network interface information.
    /// @return Optional string with the active IP address if found; otherwise, `std::nullopt`.
    std::optional<std::string> GetActiveIPAddress(const nlohmann::json& networksJson) const;

    /// @brief The agent's key.
    std::string m_key;

    /// @brief The agent's UUID.
    std::string m_uuid;

    /// @brief The agent's groups.
    std::vector<std::string> m_groups;

    /// @brief The agent's endpoint information.
    nlohmann::json m_endpointInfo;

    /// @brief The agent's metadata information.
    nlohmann::json m_metadataInfo;

    /// @brief The agent's header information.
    std::string m_headerInfo;

    /// @brief The OS information
    std::function<nlohmann::json()> m_getOSInfo;

    /// @brief The networks information
    std::function<nlohmann::json()> m_getNetworksInfo;
};
