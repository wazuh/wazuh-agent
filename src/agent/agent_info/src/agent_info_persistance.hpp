#pragma once

#include <memory>
#include <string>
#include <vector>

class Persistence;

/// @brief Manages persistence of agent information and groups in a SQLite database.
class AgentInfoPersistance
{
public:
    /// @brief Constructs the persistence manager for agent info, initializing the database and tables if necessary.
    /// @param dbFolderPath Path to the SQLite database folder.
    explicit AgentInfoPersistance(const std::string& dbFolderPath);

    /// @brief Destructor for AgentInfoPersistance.
    ~AgentInfoPersistance();

    /// @brief Deleted copy constructor.
    AgentInfoPersistance(const AgentInfoPersistance&) = delete;

    /// @brief Deleted copy assignment operator.
    AgentInfoPersistance& operator=(const AgentInfoPersistance&) = delete;

    /// @brief Deleted move constructor.
    AgentInfoPersistance(AgentInfoPersistance&&) = delete;

    /// @brief Deleted move assignment operator.
    AgentInfoPersistance& operator=(AgentInfoPersistance&&) = delete;

    /// @brief Retrieves the agent's name from the database.
    /// @return The name of the agent as a string.
    std::string GetName() const;

    /// @brief Retrieves the agent's key from the database.
    /// @return The key of the agent as a string.
    std::string GetKey() const;

    /// @brief Retrieves the agent's UUID from the database.
    /// @return The UUID of the agent as a string.
    std::string GetUUID() const;

    /// @brief Retrieves the list of agent groups from the database.
    /// @return A vector of strings, each representing a group name.
    std::vector<std::string> GetGroups() const;

    /// @brief Sets the agent's name in the database.
    /// @param name The name to set.
    void SetName(const std::string& name);

    /// @brief Sets the agent's key in the database.
    /// @param key The key to set.
    void SetKey(const std::string& key);

    /// @brief Sets the agent's UUID in the database.
    /// @param uuid The UUID to set.
    void SetUUID(const std::string& uuid);

    /// @brief Sets the agent's group list in the database, replacing any existing groups.
    /// @param groupList A vector of strings, each representing a group name.
    /// @return True if the operation was successful, false otherwise.
    bool SetGroups(const std::vector<std::string>& groupList);

    /// @brief Resets the database tables to default values, clearing all data.
    void ResetToDefault();

private:
    /// @brief Checks if the agent info table is empty.
    /// @return True if the agent info table has no entries, false otherwise.
    bool AgentInfoIsEmpty() const;

    /// @brief Creates the agent info table if it does not exist.
    void CreateAgentInfoTable();

    /// @brief Creates the agent group table if it does not exist.
    void CreateAgentGroupTable();

    /// @brief Inserts default agent information into the database.
    void InsertDefaultAgentInfo();

    /// @brief Sets a specific agent info value in the database.
    /// @param column The name of the column to set.
    /// @param value The value to set in the specified column.
    void SetAgentInfoValue(const std::string& column, const std::string& value);

    /// @brief Retrieves a specific agent info value from the database.
    /// @param column The name of the column to retrieve.
    /// @return The value from the specified column as a string.
    std::string GetAgentInfoValue(const std::string& column) const;

    /// @brief Unique pointer to the persistence instance.
    std::unique_ptr<Persistence> m_db;
};
