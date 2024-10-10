#pragma once

#include <configuration_parser.hpp>
#include <moduleWrapper.hpp>

#include <functional>
#include <string>
#include <vector>

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

        void SetGroupIdFunction(std::function<bool(const std::vector<int>&)> setGroupIdFunction);

    private:
        std::function<bool(std::vector<int>)> m_setGroupIdFunction;
    };
} // namespace centralized_configuration
