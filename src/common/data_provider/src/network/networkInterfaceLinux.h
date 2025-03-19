#pragma once

#include "inetworkInterface.h"
#include "inetworkWrapper.h"

/// @brief Linux network data retriever
class FactoryLinuxNetwork
{
public:
    /// @brief Create Linux network data retriever
    /// @param interfaceWrapper interface to retrieve network data from
    /// @return network data retriever
    static std::shared_ptr<IOSNetwork> create(const std::shared_ptr<INetworkInterfaceWrapper>& interfaceWrapper);
};

/// @brief Linux network data retriever implementation
template<unsigned short osNetworkType>
class LinuxNetworkImpl final : public IOSNetwork
{
    std::shared_ptr<INetworkInterfaceWrapper> m_interfaceAddress;

public:
    /// @brief Constructor
    /// @param interfaceAddress interface to retrieve network data from
    explicit LinuxNetworkImpl(const std::shared_ptr<INetworkInterfaceWrapper>& interfaceAddress)
        : m_interfaceAddress(interfaceAddress)
    {
    }

    /// @brief Destructor
    ~LinuxNetworkImpl() = default;

    /// @brief Fills the network information
    void buildNetworkData(nlohmann::json& /*network*/) override
    {
        throw std::runtime_error {"Non implemented specialization."};
    }
};
