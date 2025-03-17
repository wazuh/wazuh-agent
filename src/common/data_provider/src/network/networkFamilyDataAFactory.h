#pragma once

#include "networkInterfaceBSD.h"
#include "networkInterfaceLinux.h"
#include "networkInterfaceWindows.h"
#include "sharedDefs.h"
#include <memory>
#include <nlohmann/json.hpp>

/// @brief Factory for creating network data retrievers
template<OSPlatformType osType>
class FactoryNetworkFamilyCreator final
{
public:
    /// @brief Create network data retriever
    /// @return network data retriever
    static std::shared_ptr<IOSNetwork> create(const std::shared_ptr<INetworkInterfaceWrapper>& /*interface*/)
    {
        throw std::runtime_error {"Error creating network data retriever."};
    }
};

/// @brief Factory for creating Linux network data retrievers
template<>
class FactoryNetworkFamilyCreator<OSPlatformType::LINUX> final
{
public:
    /// @brief Create Linux network data retriever
    /// @return network data retriever
    static std::shared_ptr<IOSNetwork> create(const std::shared_ptr<INetworkInterfaceWrapper>& interfaceWrapper)
    {
        return FactoryLinuxNetwork::create(interfaceWrapper);
    }
};

/// @brief Factory for creating BSD network data retrievers
template<>
class FactoryNetworkFamilyCreator<OSPlatformType::BSDBASED> final
{
public:
    /// @brief Create BSD network data retriever
    /// @return network data retriever
    static std::shared_ptr<IOSNetwork> create(const std::shared_ptr<INetworkInterfaceWrapper>& interfaceWrapper)
    {
        return FactoryBSDNetwork::create(interfaceWrapper);
    }
};

/// @brief Factory for creating Windows network data retrievers
template<>
class FactoryNetworkFamilyCreator<OSPlatformType::WINDOWS> final
{
public:
    /// @brief Create Windows network data retriever
    /// @return network data retriever
    static std::shared_ptr<IOSNetwork> create(const std::shared_ptr<INetworkInterfaceWrapper>& interfaceWrapper)
    {
        return FactoryWindowsNetwork::create(interfaceWrapper);
    }
};
