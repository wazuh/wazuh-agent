#pragma once

#include <SCAPolicy.hpp>
#include <isca.hpp>

#include <command_entry.hpp>
#include <config.h>
#include <configuration_parser.hpp>
#include <dbsync.hpp>
#include <filesystem_wrapper.hpp>
#include <message.hpp>
#include <moduleWrapper.hpp>

#include <nlohmann/json.hpp>

#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

template<class DBSyncType>
class SecurityConfigurationAssessment : public ISecurityConfigurationAssessment
{
public:
    /// @brief Constructor
    /// @param configurationParser Configuration parser for setting up the module
    /// @param fileSystemWrapper File system wrapper for file operations
    SecurityConfigurationAssessment(std::shared_ptr<const configuration::ConfigurationParser> configurationParser,
                                    std::unique_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr)
        : m_fileSystemWrapper(fileSystemWrapper ? std::move(fileSystemWrapper)
                                                : std::make_unique<file_system::FileSystemWrapper>())
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
        // Execute the policies (run io context)
        // Each policy should:
        // Run regex engine, check type of policies
        // Create a report and send it to the server
    }

    /// @copydoc ISecurityConfigurationAssessment::Setup
    void Setup(std::shared_ptr<const configuration::ConfigurationParser>) override
    {
        // Read configuration
        // m_policiesFolder = configurationParser->GetConfigOrDefault(
        //     config::DEFAULT_SCA_POLICIES_PATH, "sca", "policies_path");

        // Load policies
        // AddPoliciesFromPath(m_policiesFolder);
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

private:
    /// @brief Add policies from policies path
    /// @param policiesPath Path to the policies
    void AddPoliciesFromPath(const std::filesystem::path& policiesPath)
    {
        if (!m_fileSystemWrapper->exists(policiesPath) || !m_fileSystemWrapper->is_directory(policiesPath))
        {
            return;
        }

        for (const auto& policyFile : m_fileSystemWrapper->list_directory(policiesPath))
        {
            if (!std::filesystem::is_regular_file(policyFile))
            {
                continue;
            }

            if (policyFile.extension() == ".yml" || policyFile.extension() == ".yaml")
            {
                // TODO: Load and parse the YAML file
                // SCAPolicy policy;
                // m_policies.push_back(std::move(policy));
                // enqueue policy as a task
            }
        }
    }

    const std::string SCA_DB_DISK_NAME = "sca.db";
    std::string m_name = "SCA";
    std::unique_ptr<DBSyncType> m_dBSync;
    std::string m_dbFilePath;
    std::function<int(Message)> m_pushMessage;
    std::vector<SCAPolicy> m_policies;
    std::unique_ptr<IFileSystemWrapper> m_fileSystemWrapper;
};
