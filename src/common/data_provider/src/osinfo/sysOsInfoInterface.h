#pragma once

#include <memory>
#include <nlohmann/json.hpp>
#include <string>

/// @brief Interface for OS information
class ISysOsInfoProvider
{
public:
    /// @brief Default destructor
    virtual ~ISysOsInfoProvider() = default;

    /// @brief Returns the OS name
    /// @return OS name
    virtual std::string name() const = 0;

    /// @brief Returns the OS version
    /// @return OS version
    virtual std::string version() const = 0;

    /// @brief Returns the OS major version
    /// @return OS major version
    virtual std::string majorVersion() const = 0;

    /// @brief Returns the OS minor version
    /// @return OS minor version
    virtual std::string minorVersion() const = 0;

    /// @brief Returns the OS build
    /// @return OS build
    virtual std::string build() const = 0;

    /// @brief Returns the OS release
    /// @return OS release
    virtual std::string release() const = 0;

    /// @brief Returns the OS display version
    /// @return OS display version
    virtual std::string displayVersion() const = 0;

    /// @brief Returns the OS machine
    /// @return OS machine
    virtual std::string machine() const = 0;

    /// @brief Returns the OS node name
    /// @return OS node name
    virtual std::string nodeName() const = 0;
};

/// @brief Class for OS information
class SysOsInfo
{
public:
    /// @brief Default constructor
    SysOsInfo() = default;

    /// @brief Default destructor
    ~SysOsInfo() = default;

    /// @brief Sets the OS information
    /// @param osInfoProvider OS information provider
    /// @param output Output
    static void setOsInfo(const std::shared_ptr<ISysOsInfoProvider>& osInfoProvider, nlohmann::json& output)
    {
        output["os_name"] = osInfoProvider->name();
        output["os_major"] = osInfoProvider->majorVersion();
        output["os_minor"] = osInfoProvider->minorVersion();
        output["os_build"] = osInfoProvider->build();
        output["os_version"] = osInfoProvider->version();
        output["hostname"] = osInfoProvider->nodeName();
        output["os_release"] = osInfoProvider->release();
        output["os_display_version"] = osInfoProvider->displayVersion();
        output["architecture"] = osInfoProvider->machine();
        output["os_platform"] = "windows";
        output["sysname"] = "Windows";
    }
};
