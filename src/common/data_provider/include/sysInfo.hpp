#pragma once

#include "sysInfoInterface.hpp"

constexpr auto KByte {1024};

class SysInfo : public ISysInfo
{
public:
    /// @brief Default constructor
    SysInfo() = default;

    /// @brief Default destructor
    virtual ~SysInfo() = default;

    /// @copydoc ISysInfo::hardware
    nlohmann::json hardware() override;

    /// @copydoc ISysInfo::packages
    nlohmann::json packages() override;

    /// @copydoc ISysInfo::os
    nlohmann::json os() override;

    /// @copydoc ISysInfo::processes
    nlohmann::json processes() override;

    /// @copydoc ISysInfo::networks
    nlohmann::json networks() override;

    /// @copydoc ISysInfo::ports
    nlohmann::json ports() override;

    /// @copydoc ISysInfo::hotfixes
    nlohmann::json hotfixes() override;

    /// @brief Fills the packages information using a callback
    void packages(std::function<void(nlohmann::json&)>) override;

    /// @brief Fills the processes information using a callback
    void processes(std::function<void(nlohmann::json&)>) override;

private:
    /// @brief Returns the hardware information
    /// @return Hardware information
    virtual nlohmann::json getHardware() const;

    /// @brief Returns the installed packages information
    /// @return Installed packages information
    virtual nlohmann::json getPackages() const;

    /// @brief Returns the Operating System information
    /// @return Operating System information
    virtual nlohmann::json getOsInfo() const;

    /// @brief Returns the processes information
    /// @return Processes information
    virtual nlohmann::json getProcessesInfo() const;

    /// @brief Returns the network information
    /// @return Network information
    virtual nlohmann::json getNetworks() const;

    /// @brief Returns the ports information
    /// @return Ports information
    virtual nlohmann::json getPorts() const;

    /// @brief Returns the hotfixes information
    /// @return Hotfixes information
    virtual nlohmann::json getHotfixes() const;

    /// @brief Fills the packages information using a callback
    virtual void getPackages(const std::function<void(nlohmann::json&)>&) const;

    /// @brief Fills the processes information using a callback
    virtual void getProcessesInfo(const std::function<void(nlohmann::json&)>&) const;
};
