#pragma once

#include <nlohmann/json.hpp>

class ISysInfo
{
public:
    /// @brief Default constructor
    ISysInfo() = default;

    /// @brief Default destructor
    virtual ~ISysInfo() = default;

    /// @brief Returns the hardware information
    /// @return Hardware information
    virtual nlohmann::json hardware() = 0;

    /// @brief Returns the installed packages information
    /// @return Installed packages information
    virtual nlohmann::json packages() = 0;

    /// @brief Returns the Operating System information
    /// @return Operating System information
    virtual nlohmann::json os() = 0;

    /// @brief Returns the processes information
    /// @return Processes information
    virtual nlohmann::json processes() = 0;

    /// @brief Returns the network information
    /// @return Network information
    virtual nlohmann::json networks() = 0;

    /// @brief Returns the ports information
    /// @return Ports information
    virtual nlohmann::json ports() = 0;

    /// @brief Returns the hotfixes information
    /// @return Hotfixes information
    virtual nlohmann::json hotfixes() = 0;

    /// @brief Fills the packages information using a callback
    virtual void packages(std::function<void(nlohmann::json&)>) = 0;

    /// @brief Fills the processes information using a callback
    virtual void processes(std::function<void(nlohmann::json&)>) = 0;
};
