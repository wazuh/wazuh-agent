#pragma once

#include <imodule.hpp>
#include <configuration_parser.hpp>
#include <config.h>

#include <sca.hpp>

#include <memory>
#include <string>
#include <functional>

class SCAWrapper : public IModule
{
public:
    /// @brief Constructor
    /// @param dbFolderPath Path to the database folder
    /// @param agentUUID Agent UUID
    SCAWrapper(std::string dbFolderPath,
               std::string agentUUID)
        : m_sca(std::make_shared<SecurityConfigurationAssessment>(dbFolderPath, agentUUID))
    {
    }

    /// @copydoc IModule::Run
    void Run() override
    {
        m_sca->Run();
    }

    /// @copydoc IModule::Setup
    void Setup(std::shared_ptr<const configuration::ConfigurationParser> configurationParser) override
    {
        m_sca->Setup(configurationParser);
    }

    /// @copydoc IModule::Stop
    void Stop() override
    {
        m_sca->Stop();
    }

    /// @copydoc IModule::ExecuteCommand
    Co_CommandExecutionResult ExecuteCommand(const std::string, const nlohmann::json) override
    {
        co_return module_command::CommandExecutionResult {module_command::Status::SUCCESS, "Command not implemented yet"};
    }

    /// @copydoc IModule::Name
    const std::string& Name() const override
    {
        return m_sca->Name();
    }

    /// @copydoc IModule::SetPushMessageFunction
    void SetPushMessageFunction(const std::function<int(Message)>& pushMessage) override
    {
        m_sca->SetPushMessageFunction(pushMessage);
    }

    /// @brief Enqueues an ASIO task (coroutine)
    /// @param task Task to enqueue
    void EnqueueTask(boost::asio::awaitable<void>)
    {

    }

private:
    /// @brief Pointer to the SCA module
    std::shared_ptr<SecurityConfigurationAssessment> m_sca;
};
