#pragma once

#include <SCAPolicy.hpp>

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

template<class DBSyncType = DBSync>
class SecurityConfigurationAssessment
{
public:
    /// @brief Get SCA Instance
    /// @return SCA instance
    static SecurityConfigurationAssessment&
    Instance(std::shared_ptr<configuration::ConfigurationParser> configurationParser = nullptr,
             std::unique_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr)
    {
        static SecurityConfigurationAssessment instance(configurationParser, std::move(fileSystemWrapper));
        return instance;
    }

    /// @brief Start the SCA module
    void Start()
    {
        // Execute the policies (run io context)
        // Each policy should:
        // Run regex engine, check type of policies
        // Create a report and send it to the server
    }

    /// @brief Setup the SCA module
    /// @param configurationParser Configuration parser for setting up the module
    void Setup([[maybe_unused]] std::shared_ptr<const configuration::ConfigurationParser> configurationParser)
    {
        // Read configuration
        // m_policiesFolder = configurationParser->GetConfigOrDefault(
        //     config::DEFAULT_SCA_POLICIES_PATH, "sca", "policies_path");

        // Load policies
        // AddPoliciesFromPath(m_policiesFolder);
    }

    /// @brief Stop the SCA module
    void Stop()
    {
        // Stop the policies
        // Stop the regex engine
    }

    /// @brief Execute a command
    /// @param command Command to execute
    /// @param parameters Parameters for the command
    /// @return Command execution result
    Co_CommandExecutionResult ExecuteCommand([[maybe_unused]] const std::string command,
                                             [[maybe_unused]] const nlohmann::json parameters)
    {
        co_return module_command::CommandExecutionResult {module_command::Status::FAILURE,
                                                          "SCA command handling not implemented yet"};
    }

    /// @brief Returns the name of the module
    /// @return Name of the module
    const std::string& Name() const
    {
        return m_name;
    }

    /// @brief Set the push message function
    /// @param pushMessage Function to push messages
    void SetPushMessageFunction(const std::function<int(Message)>& pushMessage)
    {
        m_pushMessage = pushMessage;
    }

private:
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

        LogError("DB file path: {}", m_dbFilePath);

        m_dBSync = std::make_unique<DBSyncType>(
            HostType::AGENT, DbEngineType::SQLITE3, m_dbFilePath, GetCreateStatement(), DbManagement::PERSISTENT);
    }

    /// @brief Destructor
    ~SecurityConfigurationAssessment() = default;

    /// @brief Deleted copy constructor
    SecurityConfigurationAssessment(const SecurityConfigurationAssessment&) = delete;

    /// @brief Deleted copy assignment operator
    SecurityConfigurationAssessment& operator=(const SecurityConfigurationAssessment&) = delete;

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

    /// @brief Get the create statement for the database
    std::string GetCreateStatement() const
    {
        // Placeholder for the actual SQL statement
        // This should be replaced with the actual SQL statement to create the SCA table
        return R"(CREATE TABLE sca (policy TEXT PRIMARY KEY );)";
    }

    const std::string SCA_DB_DISK_NAME = "sca.db";
    std::string m_name = "SCA";
    std::unique_ptr<DBSyncType> m_dBSync;
    std::string m_dbFilePath;
    std::function<int(Message)> m_pushMessage;
    std::vector<SCAPolicy> m_policies;
    std::unique_ptr<IFileSystemWrapper> m_fileSystemWrapper;
};
