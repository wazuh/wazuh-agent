#pragma once

#include "inetworkInterface.h"
#include "inetworkWrapper.h"

/// @brief Windows network data retriever
class FactoryWindowsNetwork
{
public:
    /// @brief Create Windows network data retriever
    /// @param interfaceAddress interface to retrieve network data from
    /// @return network data retriever
    static std::shared_ptr<IOSNetwork> create(const std::shared_ptr<INetworkInterfaceWrapper>& interfaceAddress);
};

/// @brief Windows network data retriever
template<int osNetworkType>
class WindowsNetworkImpl final : public IOSNetwork
{
    std::shared_ptr<INetworkInterfaceWrapper> m_interfaceAddress;

public:
    /// @brief Constructor
    /// @param interfaceAddress interface to retrieve network data from
    explicit WindowsNetworkImpl(const std::shared_ptr<INetworkInterfaceWrapper>& interfaceAddress)
        : m_interfaceAddress(interfaceAddress)
    {
    }

    /// @brief Destructor
    ~WindowsNetworkImpl() = default;

    /// @brief Fills the network information
    void buildNetworkData(nlohmann::json& /*network*/) override
    {
        throw std::runtime_error {"Non implemented specialization."};
    }
};
