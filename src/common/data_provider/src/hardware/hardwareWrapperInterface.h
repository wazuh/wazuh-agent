#pragma once

#include <cstdint>
#include <string>

/// @brief Interface for hardware data wrappers
class IOSHardwareWrapper
{
public:
    /// @brief Default destructor
    virtual ~IOSHardwareWrapper() = default;

    /// @brief Returns the board serial
    /// @return Board serial
    virtual std::string boardSerial() const = 0;

    /// @brief Returns the CPU name
    /// @return CPU name
    virtual std::string cpuName() const = 0;

    /// @brief Returns the number of CPU cores
    /// @return Number of CPU cores
    virtual int cpuCores() const = 0;

    /// @brief Returns the CPU MHz
    /// @return CPU MHz
    virtual int cpuMhz() = 0;

    /// @brief Returns the total RAM
    /// @return Total RAM
    virtual uint64_t ramTotal() const = 0;

    /// @brief Returns the free RAM
    /// @return Free RAM
    virtual uint64_t ramFree() const = 0;

    /// @brief Returns the RAM usage
    /// @return RAM usage
    virtual uint64_t ramUsage() const = 0;
};
