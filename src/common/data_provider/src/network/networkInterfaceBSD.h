#pragma once

#include "inetworkInterface.h"
#include "inetworkWrapper.h"

/// @brief BSD network data retriever
class FactoryBSDNetwork
{
public:
    /// @brief Create BSD network data retriever
    /// @param interfaceWrapper interface to retrieve network data from
    /// @return network data retriever
    static std::shared_ptr<IOSNetwork> create(const std::shared_ptr<INetworkInterfaceWrapper>& interfaceWrapper);
};

/// @brief BSD network data retriever implementation
template<unsigned short osNetworkType>
class BSDNetworkImpl final : public IOSNetwork
{
    const std::shared_ptr<INetworkInterfaceWrapper> m_interfaceAddress;

public:
    /// @brief Constructor
    /// @param interfaceAddress interface to retrieve network data from
    explicit BSDNetworkImpl(const std::shared_ptr<INetworkInterfaceWrapper>& interfaceAddress)
        : m_interfaceAddress(interfaceAddress)
    {
    }

    /// @brief Destructor
    ~BSDNetworkImpl() = default;

    /// @brief Fills the network information
    void buildNetworkData(nlohmann::json& /*network*/) override
    {
        throw std::runtime_error {"Non implemented specialization."};
    }
};
