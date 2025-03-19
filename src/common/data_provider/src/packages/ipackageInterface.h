#pragma once

#include <nlohmann/json.hpp>

/// @brief Interface for package information
class IPackage
{
public:
    /// @brief Default destructor
    virtual ~IPackage() = default;

    /// @brief Fills the package information
    /// @param package package information
    virtual void buildPackageData(nlohmann::json& package) = 0;
};
