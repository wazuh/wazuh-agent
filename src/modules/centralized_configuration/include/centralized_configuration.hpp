#pragma once

#include <configuration_parser.hpp>
#include <moduleWrapper.hpp>

#include <functional>
#include <string>

namespace centralized_configuration
{
    class CentralizedConfiguration
    {
    public:
        void Start() const;
        void Setup(const configuration::ConfigurationParser&) const;
        void Stop() const;
        Co_CommandExecutionResult ExecuteCommand(std::string command);
        std::string Name() const;
    };
} // namespace centralized_configuration
