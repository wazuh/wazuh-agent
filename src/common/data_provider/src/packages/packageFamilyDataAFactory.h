#pragma once

#include "packageMac.h"
#include "sharedDefs.h"
#include <memory>
#include <nlohmann/json.hpp>

/// @brief Factory for creating package data retrievers
template<OSPlatformType osType>
class FactoryPackageFamilyCreator final
{
public:
    /// @brief Create package data retriever
    /// @return package data retriever
    static std::shared_ptr<IPackage> create(const std::pair<PackageContext, int>& /*ctx*/)
    {
        throw std::runtime_error {"Error creating package data retriever."};
    }

    /// @brief Create package data retriever
    /// @return package data retriever
    static std::shared_ptr<IPackage> create(const std::shared_ptr<IPackageWrapper>& /*pkgwrapper*/)
    {
        throw std::runtime_error {"Error creating package data retriever."};
    }
};

/// @brief Factory for creating package data retrievers for BSD based systems
template<>
class FactoryPackageFamilyCreator<OSPlatformType::BSDBASED> final
{
public:
    /// @brief Create package data retriever for BSD based systems
    /// @param ctx package context
    static std::shared_ptr<IPackage> create(const std::pair<PackageContext, int>& ctx)
    {
        return FactoryBSDPackage::create(ctx);
    }

    /// @brief Create package data retriever for BSD based systems
    /// @param ctx package context
    static std::shared_ptr<IPackage> create(const std::pair<SQLiteLegacy::IStatement&, const int>& ctx)
    {
        return FactoryBSDPackage::create(ctx);
    }
};
