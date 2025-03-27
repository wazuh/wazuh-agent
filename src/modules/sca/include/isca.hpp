#pragma once

#include <configuration_parser.hpp>
#include <message.hpp>
#include <moduleWrapper.hpp>

#include <nlohmann/json.hpp>

#include <functional>
#include <memory>
#include <string>

class ISecurityConfigurationAssessment
{
public:
    /// @brief Virtual destructor
    virtual ~ISecurityConfigurationAssessment() = default;

    /// @brief Starts the module
    virtual void Start() = 0;

    /// @brief Configures the module
    /// @param configurationParser Configuration parser
    virtual void Setup(std::shared_ptr<const configuration::ConfigurationParser> configurationParser) = 0;

    /// @brief Stops the module
    virtual void Stop() = 0;

    /// @brief  Executes a command
    /// @param command Command to execute
    /// @param parameters A json object containing the parameters of the command to be executed
    /// @return Awaitable which will return the result of the command execution
    virtual Co_CommandExecutionResult ExecuteCommand(const std::string command, const nlohmann::json parameters) = 0;

    /// @brief Gets the name of the module
    /// @return Name of the module
    virtual const std::string& Name() const = 0;

    /// @brief Sets the push message function
    /// @param pushMessage Push message function
    virtual void SetPushMessageFunction(const std::function<int(Message)>& pushMessage) = 0;

    /// @brief Initialie the DB
    virtual void InitDb() = 0;
};
