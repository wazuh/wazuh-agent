#pragma once

#include "packages/packagesNPM.hpp"
#include "packages/packagesPYPI.hpp"
#include "sharedDefs.h"
#include <functional>
#include <map>
#include <nlohmann/json.hpp>

/// @brief Factory for modern package information
class ModernFactoryPackagesCreator final
{
public:
    /// @brief  Retrieves the modern packages information
    /// @param paths Paths to search for packages
    /// @param callback Callback function
    static void getPackages(const std::map<std::string, std::set<std::string>>& paths,
                            std::function<void(nlohmann::json&)> callback)
    {
        PYPI().getPackages(paths.at("PYPI"), callback);
        NPM().getPackages(paths.at("NPM"), callback);
    }
};
