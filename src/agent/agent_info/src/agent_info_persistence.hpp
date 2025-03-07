#pragma once

#include <iagent_info_persistence.hpp>
#include <persistence.hpp>

#include <memory>
#include <string>
#include <vector>

/// @brief Manages persistence of agent information and groups in a database.
class AgentInfoPersistence : public IAgentInfoPersistence
{
public:
    /// @brief Constructs the persistence manager for agent info, initializing the database and tables if necessary.
    /// @param dbFolderPath Path to the database folder.
    /// @param persistence Optional pointer to an existing persistence object.
    explicit AgentInfoPersistence(const std::string& dbFolderPath, std::unique_ptr<Persistence> persistence = nullptr);

    /// @brief Destructor for AgentInfoPersistence.
    ~AgentInfoPersistence();

    /// @brief Deleted copy constructor.
    AgentInfoPersistence(const AgentInfoPersistence&) = delete;

    /// @brief Deleted copy assignment operator.
    AgentInfoPersistence& operator=(const AgentInfoPersistence&) = delete;

    /// @brief Deleted move constructor.
    AgentInfoPersistence(AgentInfoPersistence&&) = delete;

    /// @brief Deleted move assignment operator.
    AgentInfoPersistence& operator=(AgentInfoPersistence&&) = delete;

    /// @copydoc IAgentInfoPersistence::GetName
    std::string GetName() const override;

    /// @copydoc IAgentInfoPersistence::GetKey
    std::string GetKey() const override;

    /// @copydoc IAgentInfoPersistence::GetUUID
    std::string GetUUID() const override;

    /// @copydoc IAgentInfoPersistence::GetGroups
    std::vector<std::string> GetGroups() const override;

    /// @copydoc IAgentInfoPersistence::SetName
    bool SetName(const std::string& name) override;

    /// @copydoc IAgentInfoPersistence::SetKey
    bool SetKey(const std::string& key) override;

    /// @copydoc IAgentInfoPersistence::SetUUID
    bool SetUUID(const std::string& uuid) override;

    /// @copydoc IAgentInfoPersistence::SetGroups
    bool SetGroups(const std::vector<std::string>& groupList) override;

    /// @copydoc IAgentInfoPersistence::ResetToDefault
    bool ResetToDefault() override;

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
    /// @return True if the operation was successful, false otherwise.
    bool SetAgentInfoValue(const std::string& column, const std::string& value);

    /// @brief Retrieves a specific agent info value from the database.
    /// @param column The name of the column to retrieve.
    /// @return The value from the specified column as a string.
    std::string GetAgentInfoValue(const std::string& column) const;

    /// @brief Unique pointer to the persistence instance.
    std::unique_ptr<Persistence> m_db;
};
