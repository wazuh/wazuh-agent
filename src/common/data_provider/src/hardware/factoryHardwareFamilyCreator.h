#pragma once

#include "hardwareImplMac.h"
#include "hardwareInterface.h"
#include "hardwareWrapperInterface.h"
#include "sharedDefs.h"
#include <memory>
#include <nlohmann/json.hpp>

/// @brief Factory for creating hardware data retrievers
template<OSPlatformType osType>
class FactoryHardwareFamilyCreator final
{
public:
    /// @brief Create hardware data retriever
    /// @return hardware data retriever
    static std::shared_ptr<IOSHardware> create(const std::shared_ptr<IOSHardwareWrapper>& /*wrapperInterface*/)
    {
        throw std::runtime_error {"Error creating network data retriever."};
    }
};

/// @brief Factory for creating hardware data retrievers for BSD based systems
template<>
class FactoryHardwareFamilyCreator<OSPlatformType::BSDBASED> final
{
public:
    /// @brief Create hardware data retriever for BSD based systems
    /// @param wrapperInterface hardware wrapper interface
    /// @return hardware data retriever
    static std::shared_ptr<IOSHardware> create(const std::shared_ptr<IOSHardwareWrapper>& wrapperInterface)
    {
        return FactoryBSDHardware::create(wrapperInterface);
    }
};
