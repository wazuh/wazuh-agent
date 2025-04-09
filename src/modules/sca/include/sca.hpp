#pragma once

#include <sca_policy.hpp>

#include <command_entry.hpp>
#include <config.h>
#include <configuration_parser.hpp>
#include <dbsync.hpp>
#include <filesystem_wrapper.hpp>
#include <imodule.hpp>
#include <message.hpp>
#include <sca_policy_loader.hpp>

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

#include <filesystem>
#include <functional>
#include <iostream>
#include <memory>
#include <string>
#include <vector>

template<class DBSyncType = DBSync>
class SecurityConfigurationAssessment : public IModule
{
public:
    /// @brief Constructor
    /// @param configurationParser Configuration parser for setting up the module
    /// @param fileSystemWrapper File system wrapper for file operations
    SecurityConfigurationAssessment(std::shared_ptr<const configuration::ConfigurationParser> configurationParser,
                                    std::shared_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr)
        : m_fileSystemWrapper(fileSystemWrapper ? std::move(fileSystemWrapper)
                                                : std::make_shared<file_system::FileSystemWrapper>())
    {
        Setup(configurationParser);
    }

    /// @brief Destructor
    ~SecurityConfigurationAssessment() = default;

    /// @brief Deleted copy constructor
    SecurityConfigurationAssessment(const SecurityConfigurationAssessment&) = delete;

    /// @brief Deleted copy assignment operator
    SecurityConfigurationAssessment& operator=(const SecurityConfigurationAssessment&) = delete;

    /// @copydoc IModule::Run
    void Run() override
    {
        // Execute the policies (run io context)
        // Each policy should:
        // Run regex engine, check type of policies
        // Create a report and send it to the server
        m_ioContext.run();
    }

    /// @copydoc IModule::Setup
    void Setup(std::shared_ptr<const configuration::ConfigurationParser> configurationParser) override
    {
        m_dbFilePath = configurationParser->GetConfigOrDefault(config::DEFAULT_DATA_PATH, "agent", "path.data") + "/" +
                       SCA_DB_DISK_NAME;
        m_enabled = configurationParser->GetConfigOrDefault(config::sca::DEFAULT_ENABLED, "sca", "enabled");
        m_scanOnStart =
            configurationParser->GetConfigOrDefault(config::sca::DEFAULT_SCAN_ON_START, "sca", "scan_on_start");
        m_scanInterval = configurationParser->GetTimeConfigOrDefault(config::sca::DEFAULT_INTERVAL, "sca", "interval");

        m_policies = [this, &configurationParser]()
        {
            SCAPolicyLoader policyLoader(m_fileSystemWrapper, configurationParser, m_pushMessage);
            return policyLoader.GetPolicies();
        }();

        m_dBSync = std::make_unique<DBSyncType>(
            HostType::AGENT, DbEngineType::SQLITE3, m_dbFilePath, GetCreateStatement(), DbManagement::PERSISTENT);
    }

    /// @copydoc IModule::Stop
    void Stop() override
    {
        // Stop the policies
        // Stop the regex engine
        m_ioContext.stop();
    }

    /// @copydoc IModule::ExecuteCommand
    Co_CommandExecutionResult ExecuteCommand([[maybe_unused]] const std::string command,
                                             [[maybe_unused]] const nlohmann::json parameters) override
    {
        if (!m_enabled)
        {
            LogInfo("SCA module is disabled.");
            co_return module_command::CommandExecutionResult {module_command::Status::FAILURE, "Module is disabled"};
        }

        LogInfo("Command: {}", command);
        co_return module_command::CommandExecutionResult {module_command::Status::SUCCESS,
                                                          "Command not implemented yet"};
    }

    /// @copydoc IModule::Name
    const std::string& Name() const override
    {
        return m_name;
    }

    /// @copydoc IModule::SetPushMessageFunction
    void SetPushMessageFunction(const std::function<int(Message)>& pushMessage) override
    {
        m_pushMessage = pushMessage;
    }

    /// @brief Enqueues an ASIO task (coroutine)
    /// @param task Task to enqueue
    void EnqueueTask(boost::asio::awaitable<void> task)
    {
        // NOLINTBEGIN(cppcoreguidelines-avoid-capturing-lambda-coroutines)
        boost::asio::co_spawn(
            m_ioContext,
            [task = std::move(task)]() mutable -> boost::asio::awaitable<void>
            {
                try
                {
                    co_await std::move(task);
                }
                catch (const std::exception& e)
                {
                    LogError("Logcollector coroutine task exited with an exception: {}", e.what());
                }
            },
            boost::asio::detached);
        // NOLINTEND(cppcoreguidelines-avoid-capturing-lambda-coroutines)
    }

private:
    /// @brief Get the create statement for the database
    std::string GetCreateStatement() const
    {
        // Placeholder for the actual SQL statement
        // This should be replaced with the actual SQL statement to create the SCA table
        return R"(CREATE TABLE sca (policy TEXT PRIMARY KEY );)";
    }

    /// @brief SCA db file name
    const std::string SCA_DB_DISK_NAME = "sca.db";

    /// @brief SCA module name
    std::string m_name = "SCA";

    /// @brief Pointer to DBSync
    std::unique_ptr<DBSyncType> m_dBSync;

    /// @brief Path to the database file
    std::string m_dbFilePath;

    /// @brief Function for pushing event messages
    std::function<int(Message)> m_pushMessage;

    /// @brief Pointer to a file system wrapper
    std::shared_ptr<IFileSystemWrapper> m_fileSystemWrapper;

    /// @brief Flag indicating whether the module is enabled
    bool m_enabled;

    /// @brief Flag indicating whether to scan on start
    bool m_scanOnStart;

    /// @brief Scan interval in seconds
    std::time_t m_scanInterval;

    /// @brief List of policies
    std::vector<SCAPolicy> m_policies;

    /// @brief Boost ASIO context
    boost::asio::io_context m_ioContext;
};
