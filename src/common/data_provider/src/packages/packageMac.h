#pragma once

#include "ipackageInterface.h"
#include "ipackageWrapper.h"
#include "isqliteWrapper.hpp"

/// @brief Package information
struct PackageContext
{
    std::string filePath;
    std::string package;
    std::string version;
};

/// @brief Factory for package information
class FactoryBSDPackage
{
public:
    /// @brief Creates a package data retriever
    /// @param ctx package context
    /// @return package data retriever
    static std::shared_ptr<IPackage> create(const std::pair<PackageContext, int>& ctx);

    /// @brief Creates a package data retriever
    /// @param ctx package context
    /// @return package data retriever
    static std::shared_ptr<IPackage> create(const std::pair<SQLiteLegacy::IStatement&, const int>& ctx);
};

/// @brief Package information
class BSDPackageImpl final : public IPackage
{
    const std::shared_ptr<IPackageWrapper> m_packageWrapper;

public:
    /// @brief Constructor
    /// @param packageWrapper package wrapper
    explicit BSDPackageImpl(const std::shared_ptr<IPackageWrapper>& packageWrapper);

    /// @copydoc IPackage::buildPackageData
    void buildPackageData(nlohmann::json& package) override;
};
