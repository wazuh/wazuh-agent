#pragma once

#include <string>
#include <vector>

/// @brief Interface for managing agent information.
class IAgentInfo
{
public:
    virtual ~IAgentInfo() = default;

    /// @brief Gets the agent's name.
    /// @return The agent's name.
    virtual std::string GetName() const = 0;

    /// @brief Gets the agent's key.
    /// @return The agent's key.
    virtual std::string GetKey() const = 0;

    /// @brief Gets the agent's UUID.
    /// @return The agent's UUID.
    virtual std::string GetUUID() const = 0;

    /// @brief Gets the agent's groups.
    /// @return A vector of the agent's groups.
    virtual std::vector<std::string> GetGroups() const = 0;

    /// @brief Sets the agent's name. The change is not saved to the database until `Save` is called.
    /// @param name The agent's new name.
    /// @return True if the name was successfully set, false otherwise.
    virtual bool SetName(const std::string& name) = 0;

    /// @brief Sets the agent's key. The change is not saved to the database until `Save` is called.
    /// @param key The agent's new key.
    /// @return True if the key was successfully set, false otherwise.
    virtual bool SetKey(const std::string& key) = 0;

    /// @brief Sets the agent's UUID. The change is not saved to the database until `Save` is called.
    /// @param uuid The agent's new UUID.
    virtual void SetUUID(const std::string& uuid) = 0;

    /// @brief Sets the agent's groups. The change is not saved to the database until `Save` is called.
    /// @param groupList A vector of the agent's new groups.
    virtual void SetGroups(const std::vector<std::string>& groupList) = 0;

    /// @brief Gets the agent's type.
    /// @return The agent's type.
    virtual std::string GetType() const = 0;

    /// @brief Gets the agent's version.
    /// @return The agent's version.
    virtual std::string GetVersion() const = 0;

    /// @brief Gets the agent information for the request header.
    /// @return A string with the information for the request header.
    virtual std::string GetHeaderInfo() const = 0;

    /// @brief Gets all the information about the agent.
    /// @return A string with all information about the agent.
    virtual std::string GetMetadataInfo() const = 0;

    /// @brief Restores and saves the agent's information to the database.
    virtual void Save() const = 0;

    /// @brief Saves the agent's group information to the database.
    /// @return True if the operation was successful, false otherwise.
    virtual bool SaveGroups() const = 0;
};
