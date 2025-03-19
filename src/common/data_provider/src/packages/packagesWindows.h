#pragma once

// clang-format off
#include <winsock2.h>
#include <windows.h>
#include <set>
// clang-format on

#include "ipackageInterface.h"
#include "ipackageWrapper.h"

/// @brief Factory for Windows packages
class FactoryWindowsPackage
{
public:
    /// @brief Creates a Windows package data retriever
    /// @param key registry key
    /// @param userId user id
    /// @param nameApp name of the application
    /// @param cacheRegistry cache registry
    /// @return Windows package data retriever
    static std::shared_ptr<IPackage> create(const HKEY key,
                                            const std::string& userId,
                                            const std::string& nameApp,
                                            const std::set<std::string>& cacheRegistry);
};

/// @brief Package information
class WindowsPackageImpl final : public IPackage
{
private:
    const std::shared_ptr<IPackageWrapper> m_packageWrapper;

public:
    /// @brief Constructor
    /// @param packageWrapper package wrapper
    explicit WindowsPackageImpl(const std::shared_ptr<IPackageWrapper>& packageWrapper);

    /// @copydoc IPackage::buildPackageData
    void buildPackageData(nlohmann::json& package) override;
};
