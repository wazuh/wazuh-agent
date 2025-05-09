#pragma once

#include <isca_policy.hpp>

#include <command_entry.hpp>
#include <configuration_parser.hpp>
#include <idbsync.hpp>
#include <ifilesystem_wrapper.hpp>
#include <imodule.hpp>
#include <message.hpp>

#include <boost/asio.hpp>
#include <nlohmann/json.hpp>

#include <functional>
#include <memory>
#include <string>
#include <vector>

class SecurityConfigurationAssessment : public IModule
{
public:
    /// @brief Constructor
    /// @param configurationParser Configuration parser for setting up the module
    /// @param agentUUID Agent UUID
    /// @param dbSync Pointer to IDBSync for database synchronization
    /// @param fileSystemWrapper File system wrapper for file operations
    SecurityConfigurationAssessment(std::shared_ptr<const configuration::ConfigurationParser> configurationParser,
                                    std::string agentUUID,
                                    std::shared_ptr<IDBSync> dbSync = nullptr,
                                    std::shared_ptr<IFileSystemWrapper> fileSystemWrapper = nullptr);

    /// @brief Destructor
    ~SecurityConfigurationAssessment() = default;

    /// @brief Deleted copy constructor
    SecurityConfigurationAssessment(const SecurityConfigurationAssessment&) = delete;

    /// @brief Deleted copy assignment operator
    SecurityConfigurationAssessment& operator=(const SecurityConfigurationAssessment&) = delete;

    /// @copydoc IModule::Run
    void Run() override;

    /// @copydoc IModule::Setup
    void Setup(std::shared_ptr<const configuration::ConfigurationParser> configurationParser) override;

    /// @copydoc IModule::Stop
    void Stop() override;

    /// @copydoc IModule::ExecuteCommand
    Co_CommandExecutionResult ExecuteCommand(const std::string command, const nlohmann::json parameters) override;

    /// @copydoc IModule::Name
    const std::string& Name() const override;

    /// @copydoc IModule::SetPushMessageFunction
    void SetPushMessageFunction(const std::function<int(Message)>& pushMessage) override;

    /// @brief Enqueues an ASIO task (coroutine)
    /// @param task Task to enqueue
    void EnqueueTask(boost::asio::awaitable<void> task);

private:
    /// @brief Get the create statement for the database
    std::string GetCreateStatement() const;

    /// @brief SCA db file name
    const std::string SCA_DB_DISK_NAME = "sca.db";

    /// @brief SCA module name
    std::string m_name = "SCA";

    /// @brief Agent UUID
    std::string m_agentUUID {""};

    /// @brief Pointer to IDBSync
    std::shared_ptr<IDBSync> m_dBSync;

    /// @brief Function for pushing event messages
    std::function<int(Message)> m_pushMessage;

    /// @brief Pointer to a file system wrapper
    std::shared_ptr<IFileSystemWrapper> m_fileSystemWrapper;

    /// @brief Flag indicating whether the module is enabled
    bool m_enabled = true;

    /// @brief Flag indicating whether to scan on start
    bool m_scanOnStart = true;

    /// @brief Scan interval in seconds
    std::time_t m_scanInterval = 3600;

    /// @brief List of policies
    std::vector<std::unique_ptr<ISCAPolicy>> m_policies;

    /// @brief Boost ASIO context
    std::unique_ptr<boost::asio::io_context> m_ioContext;
};
