#pragma once

#include "sysOsInfoInterface.h"

/// @brief Class for Windows OS information
class SysOsInfoProviderWindows final : public ISysOsInfoProvider
{
public:
    /// @brief Constructor
    SysOsInfoProviderWindows();

    /// @brief Default destructor
    ~SysOsInfoProviderWindows() = default;

    /// @copydoc ISysOsInfoProvider::name
    std::string name() const override;

    /// @copydoc ISysOsInfoProvider::version
    std::string version() const override;

    /// @copydoc ISysOsInfoProvider::majorVersion
    std::string majorVersion() const override;

    /// @copydoc ISysOsInfoProvider::minorVersion
    std::string minorVersion() const override;

    /// @copydoc ISysOsInfoProvider::build
    std::string build() const override;

    /// @copydoc ISysOsInfoProvider::release
    std::string release() const override;

    /// @copydoc ISysOsInfoProvider::displayVersion
    std::string displayVersion() const override;

    /// @copydoc ISysOsInfoProvider::machine
    std::string machine() const override;

    /// @copydoc ISysOsInfoProvider::nodeName
    std::string nodeName() const override;

private:
    const std::string m_majorVersion;
    const std::string m_minorVersion;
    const std::string m_build;
    const std::string m_buildRevision;
    const std::string m_version;
    const std::string m_release;
    const std::string m_displayVersion;
    const std::string m_name;
    const std::string m_machine;
    const std::string m_nodeName;
};
