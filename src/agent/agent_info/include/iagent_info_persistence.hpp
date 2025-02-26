#pragma once

#include <memory>
#include <string>
#include <vector>

/// @brief Interface for managing persistence of agent information and groups in a database.
class IAgentInfoPersistence
{
public:
    virtual ~IAgentInfoPersistence() = default;

    /// @brief Retrieves the agent's name from the database.
    /// @return The name of the agent as a string.
    virtual std::string GetName() const = 0;

    /// @brief Retrieves the agent's key from the database.
    /// @return The key of the agent as a string.
    virtual std::string GetKey() const = 0;

    /// @brief Retrieves the agent's UUID from the database.
    /// @return The UUID of the agent as a string.
    virtual std::string GetUUID() const = 0;

    /// @brief Retrieves the list of agent groups from the database.
    /// @return A vector of strings, each representing a group name.
    virtual std::vector<std::string> GetGroups() const = 0;

    /// @brief Sets the agent's name in the database.
    /// @param name The name to set.
    /// @return True if the operation was successful, false otherwise.
    virtual bool SetName(const std::string& name) = 0;

    /// @brief Sets the agent's key in the database.
    /// @param key The key to set.
    /// @return True if the operation was successful, false otherwise.
    virtual bool SetKey(const std::string& key) = 0;

    /// @brief Sets the agent's UUID in the database.
    /// @param uuid The UUID to set.
    /// @return True if the operation was successful, false otherwise.
    virtual bool SetUUID(const std::string& uuid) = 0;

    /// @brief Sets the agent's group list in the database, replacing any existing groups.
    /// @param groupList A vector of strings, each representing a group name.
    /// @return True if the operation was successful, false otherwise.
    virtual bool SetGroups(const std::vector<std::string>& groupList) = 0;

    /// @brief Resets the database tables to default values, clearing all data.
    /// @return True if the reset was successful, false otherwise.
    virtual bool ResetToDefault() = 0;
};
