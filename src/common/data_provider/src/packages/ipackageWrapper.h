#pragma once

#include "ipackageInterface.h"

/// @brief Interface for package information
class IPackageWrapper
{
public:
    /// @brief Default destructor
    virtual ~IPackageWrapper() = default;

    /// @brief Returns the package name
    /// @param package Package
    virtual void name(nlohmann::json& package) const = 0;

    /// @brief Returns the package version
    /// @param package Package
    virtual void version(nlohmann::json& package) const = 0;

    /// @brief Returns the package groups
    /// @param package Package
    virtual void groups(nlohmann::json& package) const = 0;

    /// @brief Returns the package description
    /// @param package Package
    virtual void description(nlohmann::json& package) const = 0;

    /// @brief Returns the package architecture
    /// @param package Package
    virtual void architecture(nlohmann::json& package) const = 0;

    /// @brief Returns the package format
    /// @param package Package
    virtual void format(nlohmann::json& package) const = 0;

    /// @brief Returns the package os patch
    /// @param package Package
    virtual void osPatch(nlohmann::json& package) const = 0;

    /// @brief Returns the package source
    /// @param package Package
    virtual void source(nlohmann::json& package) const = 0;

    /// @brief Returns the package location
    /// @param package Package
    virtual void location(nlohmann::json& package) const = 0;

    /// @brief Returns the package priority
    /// @param package Package
    virtual void priority(nlohmann::json& package) const = 0;

    /// @brief Returns the package size
    /// @param package Package
    virtual void size(nlohmann::json& package) const = 0;

    /// @brief Returns the package vendor
    /// @param package Package
    virtual void vendor(nlohmann::json& package) const = 0;

    /// @brief Returns the package install time
    /// @param package Package
    virtual void install_time(nlohmann::json& package) const = 0;

    /// @brief Returns the package multiarch
    /// @param package Package
    virtual void multiarch(nlohmann::json& package) const = 0;
};
