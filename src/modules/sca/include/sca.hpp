#pragma once

#include <isca.hpp>

#include <configuration_parser.hpp>
#include <message.hpp>
#include <moduleWrapper.hpp>

#include <nlohmann/json.hpp>

#include <functional>
#include <memory>
#include <string>

class ISecurityConfigurationAssessment : public ISecurityConfigurationAssessment
{
public:
    /// @copydoc ISecurityConfigurationAssessment::~ISecurityConfigurationAssessment
    virtual ~ISecurityConfigurationAssessment() = default;

    /// @copydoc ISecurityConfigurationAssessment::Start
    virtual void Start() override;

    /// @copydoc ISecurityConfigurationAssessment::Setup
    virtual void Setup(std::shared_ptr<const configuration::ConfigurationParser> configurationParser) override;

    /// @copydoc ISecurityConfigurationAssessment::Stop
    virtual void Stop() override;

    /// @copydoc ISecurityConfigurationAssessment::ExecuteCommand
    virtual Co_CommandExecutionResult ExecuteCommand(const std::string command,
                                                     const nlohmann::json parameters) override;

    /// @copydoc ISecurityConfigurationAssessment::Name
    virtual const std::string& Name() const override;

    /// @copydoc ISecurityConfigurationAssessment::SetPushMessageFunction
    virtual void SetPushMessageFunction(const std::function<int(Message)>& pushMessage) override;

    /// @copydoc ISecurityConfigurationAssessment::InitDb
    virtual void InitDb() override;
};
