#pragma once

#include <sqlite_manager.hpp>

#include <memory>
#include <string>
#include <vector>

static constexpr char sqlitedb_path[] = "agent_info.db";

/// @brief Manages persistence of agent information and groups in a SQLite database.
class AgentInfoPersistance
{
public:
    /// @brief Constructs the persistence manager for agent info, initializing the database and tables if necessary.
    /// @param dbPath Path to the SQLite database file.
    explicit AgentInfoPersistance(const std::string& dbPath = sqlitedb_path);

    /// @brief Destructor for AgentInfoPersistance.
    ~AgentInfoPersistance();

    AgentInfoPersistance(const AgentInfoPersistance&) = delete;
    AgentInfoPersistance& operator=(const AgentInfoPersistance&) = delete;
    AgentInfoPersistance(AgentInfoPersistance&&) = delete;
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
    void SetGroups(const std::vector<std::string>& groupList);

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

    /// @brief Unique pointer to the SQLite database manager instance.
    std::unique_ptr<sqlite_manager::SQLiteManager> m_db;
};
