#pragma once

#include <nlohmann/json.hpp>

/// @brief Interface for hardware data retrievers
class IOSHardware
{
public:
    /// @brief Default destructor
    virtual ~IOSHardware() = default;

    /// @brief Fills the hardware information
    /// @param hardware hardware information
    virtual void buildHardwareData(nlohmann::json& hardware) = 0;
};
