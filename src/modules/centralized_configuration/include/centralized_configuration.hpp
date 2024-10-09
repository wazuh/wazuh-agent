#pragma once

#include <configuration_parser.hpp>

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
        std::string Name() const;
    };
} // namespace centralized_configuration
