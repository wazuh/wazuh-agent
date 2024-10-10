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

        void SetGroupIdFunction(std::function<void(const std::vector<std::string>&)> setGroupIdFunction);

    private:
        std::function<void(const std::vector<std::string>&)> m_setGroupIdFunction;
    };
} // namespace centralized_configuration
