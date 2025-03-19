#pragma once

#include <memory>
#include <nlohmann/json.hpp>

/// @brief Interface for ports
class IOSPort
{
public:
    /// @brief Default destructor
    virtual ~IOSPort() = default;

    /// @brief Fills the port information
    /// @param port port information
    virtual void buildPortData(nlohmann::json& port) = 0;
};
