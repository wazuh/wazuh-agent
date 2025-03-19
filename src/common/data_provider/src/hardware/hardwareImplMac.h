#pragma once

#include "hardwareInterface.h"
#include "hardwareWrapperInterface.h"

/// @brief MacOS hardware data retriever
class OSHardwareMac final : public IOSHardware
{
    std::shared_ptr<IOSHardwareWrapper> m_wrapper;

public:
    /// @brief Constructor
    /// @param wrapper hardware wrapper
    explicit OSHardwareMac(const std::shared_ptr<IOSHardwareWrapper>& wrapper)
        : m_wrapper(wrapper)
    {
    }

    /// @brief Default destructor
    ~OSHardwareMac() = default;

    /// @copydoc IOSHardware::buildHardwareData
    void buildHardwareData(nlohmann::json& hardware) override
    {
        hardware["board_serial"] = m_wrapper->boardSerial();
        hardware["cpu_name"] = m_wrapper->cpuName();
        hardware["cpu_cores"] = m_wrapper->cpuCores();
        hardware["cpu_mhz"] = m_wrapper->cpuMhz();
        hardware["ram_total"] = m_wrapper->ramTotal();
        hardware["ram_free"] = m_wrapper->ramFree();
        hardware["ram_usage"] = m_wrapper->ramUsage();
    }
};

/// @brief Factory for creating MacOS hardware data retrievers
class FactoryBSDHardware final
{
public:
    /// @brief Create MacOS hardware data retriever
    /// @param wrapper hardware wrapper
    /// @return hardware data retriever
    static std::shared_ptr<IOSHardware> create(const std::shared_ptr<IOSHardwareWrapper>& wrapper)
    {
        return std::make_shared<OSHardwareMac>(wrapper);
    }
};
