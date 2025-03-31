#pragma once

#include <isca.hpp>

#include <command_entry.hpp>
#include <config.h>
#include <configuration_parser.hpp>
#include <dbsync.hpp>
#include <message.hpp>
#include <moduleWrapper.hpp>

#include <nlohmann/json.hpp>

#include <functional>
#include <memory>
#include <string>

template<class DBSyncType>
class SecurityConfigurationAssessment : public ISecurityConfigurationAssessment
{
public:
    SecurityConfigurationAssessment(std::shared_ptr<const configuration::ConfigurationParser> configurationParser)
    {
        m_dbFilePath = configurationParser->GetConfigOrDefault(config::DEFAULT_DATA_PATH, "agent", "path.data") + "/" +
                       SCA_DB_DISK_NAME;

        m_dBSync = std::make_unique<DBSyncType>(
            HostType::AGENT, DbEngineType::SQLITE3, m_dbFilePath, "", DbManagement::PERSISTENT);
    }

    /// @copydoc ISecurityConfigurationAssessment::~ISecurityConfigurationAssessment
    ~SecurityConfigurationAssessment() = default;
    SecurityConfigurationAssessment(const SecurityConfigurationAssessment&) = delete;
    SecurityConfigurationAssessment& operator=(const SecurityConfigurationAssessment&) = delete;

    /// @copydoc ISecurityConfigurationAssessment::Start
    void Start() override
    {
        // Execute the policies
        // Run regex engine, check type of policies
        // Run the policies
        // Create a report and send it to the server
    }

    /// @copydoc ISecurityConfigurationAssessment::Setup
    void Setup(std::shared_ptr<const configuration::ConfigurationParser>) override
    {
        // Read configuration
        // Load policies
        // Validate requirements
    }

    /// @copydoc ISecurityConfigurationAssessment::Stop
    void Stop() override
    {
        // Stop the policies
        // Stop the regex engine
    }

    /// @copydoc ISecurityConfigurationAssessment::ExecuteCommand
    Co_CommandExecutionResult ExecuteCommand([[maybe_unused]] const std::string command,
                                             [[maybe_unused]] const nlohmann::json parameters) override
    {
        co_return module_command::CommandExecutionResult {module_command::Status::FAILURE,
                                                          "SCA command handling not implemented yet"};
    }

    /// @copydoc ISecurityConfigurationAssessment::Name
    const std::string& Name() const override
    {
        return m_name;
    }

    /// @copydoc ISecurityConfigurationAssessment::SetPushMessageFunction
    void SetPushMessageFunction(const std::function<int(Message)>& pushMessage) override
    {
        m_pushMessage = pushMessage;
    }

    /// @copydoc ISecurityConfigurationAssessment::InitDb
    void InitDb() override {}

private:
    const std::string SCA_DB_DISK_NAME = "sca.db";
    std::string m_name = "SCA";
    std::unique_ptr<DBSyncType> m_dBSync;
    std::string m_dbFilePath;
    std::function<int(Message)> m_pushMessage;
};
