#pragma once

#include <isca.hpp>

#include <configuration_parser.hpp>
#include <message.hpp>
#include <moduleWrapper.hpp>

#include <nlohmann/json.hpp>

#include <functional>
#include <memory>
#include <string>

class SecurityConfigurationAssessment : public ISecurityConfigurationAssessment
{
public:
    /// @brief Get the instance of the SecurityConfigurationAssessment
    /// @return The instance of the SecurityConfigurationAssessment
    static SecurityConfigurationAssessment& Instance()
    {
        static SecurityConfigurationAssessment s_instance;
        return s_instance;
    }

    /// @copydoc ISecurityConfigurationAssessment::~ISecurityConfigurationAssessment

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
    Co_CommandExecutionResult ExecuteCommand(const std::string, const nlohmann::json) override
    {
        return {};
    }

    /// @copydoc ISecurityConfigurationAssessment::Name
    const std::string& Name() const override
    {
        return m_name;
    }

    /// @copydoc ISecurityConfigurationAssessment::SetPushMessageFunction
    void SetPushMessageFunction(const std::function<int(Message)>&) override {}

    /// @copydoc ISecurityConfigurationAssessment::InitDb
    void InitDb() override
    {
        // Initialize the database
    }

private:
    SecurityConfigurationAssessment() = default;
    ~SecurityConfigurationAssessment() = default;
    SecurityConfigurationAssessment(const SecurityConfigurationAssessment&) = delete;
    SecurityConfigurationAssessment& operator=(const SecurityConfigurationAssessment&) = delete;

    std::string m_name = "SCA";
};
