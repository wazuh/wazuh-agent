#pragma once

#include <string>

namespace centralized_configuration
{
    class CentralizedConfiguration
    {
    public:
        void Start() const;
        void Stop() const;
        std::string Name() const;
    };
} // namespace centralized_configuration
