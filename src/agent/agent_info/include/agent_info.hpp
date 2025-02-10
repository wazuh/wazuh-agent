#pragma once

#include <config.h>
#include <nlohmann/json.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

class AgentInfoPersistance;

/// @brief Stores and manages information about an agent.
///
/// This class provides methods for getting and setting the agent's name, key,
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
    /// @param dbFolderPath Path to the database folder.
    /// @param getOSInfo Function to retrieve OS information in JSON format.
    /// @param getNetworksInfo Function to retrieve network information in JSON format.
    /// @param agentIsRegistering True if the agent is being registered, false otherwise.
    /// @param persistence Optional pointer to an AgentInfoPersistance object.
    AgentInfo(const std::string& dbFolderPath = config::DEFAULT_DATA_PATH,
              std::function<nlohmann::json()> getOSInfo = nullptr,
              std::function<nlohmann::json()> getNetworksInfo = nullptr,
              bool agentIsRegistering = false,
              std::shared_ptr<AgentInfoPersistance> persistence = nullptr);

    /// @brief Gets the agent's name.
    /// @return The agent's name.
    std::string GetName() const;

    /// @brief Gets the agent's key.
    /// @return The agent's key.
    std::string GetKey() const;

    /// @brief Gets the agent's UUID.
    /// @return The agent's UUID.
    std::string GetUUID() const;

    /// @brief Gets the agent's groups.
    /// @return A vector of the agent's groups.
    std::vector<std::string> GetGroups() const;

    /// @brief Sets the agent's name. The change is not saved to the database until `Save` is called.
    /// @param name The agent's new name.
    /// @return True if the name was successfully set, false otherwise.
    bool SetName(const std::string& name);

    /// @brief Sets the agent's key. The change is not saved to the database until `Save` is called.
    /// @param key The agent's new key.
    /// @return True if the key was successfully set, false otherwise.
    bool SetKey(const std::string& key);

    /// @brief Sets the agent's UUID. The change is not saved to the database until `Save` is called.
    /// @param uuid The agent's new UUID.
    void SetUUID(const std::string& uuid);

    /// @brief Sets the agent's groups. The change is not saved to the database until `Save` is called.
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
    /// @return A string with all information about the agent.
    std::string GetMetadataInfo() const;

    /// @brief Restores and saves the agent's information to the database.
    void Save() const;

    /// @brief Saves the agent's group information to the database.
    /// @return True if the operation was successful, false otherwise.
    bool SaveGroups() const;

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

    /// @brief Loads the header information into `m_headerInfo`.
    void LoadHeaderInfo();

    /// @brief Extracts the active IP address from the network JSON data.
    /// @param networksJson JSON object containing network interface information.
    /// @return Vector of strings with the active IP addresses.
    std::vector<std::string> GetActiveIPAddresses(const nlohmann::json& networksJson) const;

    /// @brief The agent's name.
    std::string m_name;

    /// @brief The agent's key.
    std::string m_key;

    /// @brief The agent's UUID.
    std::string m_uuid;

    /// @brief The agent's groups.
    std::vector<std::string> m_groups;

    /// @brief The agent's endpoint information.
    nlohmann::json m_endpointInfo;

    /// @brief The agent's header information.
    std::string m_headerInfo;

    /// @brief The OS information
    std::function<nlohmann::json()> m_getOSInfo;

    /// @brief The networks information
    std::function<nlohmann::json()> m_getNetworksInfo;

    /// @brief Specify if the agent is about to register.
    bool m_agentIsRegistering;

    /// @brief Pointer to the agent info persistence instance.
    std::shared_ptr<AgentInfoPersistance> m_persistence;
};
