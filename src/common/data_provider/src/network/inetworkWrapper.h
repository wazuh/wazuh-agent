#pragma once

#include "inetworkInterface.h"
#include <optional>

/// @brief Interface for network data wrappers
class INetworkInterfaceWrapper
{
public:
    /// @brief Default destructor
    virtual ~INetworkInterfaceWrapper() = default;

    /// @brief Returns the network family
    /// @return Network family
    virtual int family() const = 0;

    /// @brief Returns the network name
    /// @return Network name
    virtual std::string name() const = 0;

    /// @brief Returns the network adapter
    /// @param network Network
    virtual void adapter(nlohmann::json& network) const = 0;

    /// @brief Returns the network address
    /// @return Network address
    virtual std::string address() const = 0;

    /// @brief Returns the network netmask
    /// @return Network netmask
    virtual std::string netmask() const = 0;

    /// @brief Returns the network broadcast
    /// @param network Network
    virtual void broadcast(nlohmann::json& network) const = 0;

    /// @brief Returns the network address v6
    /// @return Network address v6
    virtual std::string addressV6() const = 0;

    /// @brief Returns the network netmask v6
    /// @return Network netmask v6
    virtual std::string netmaskV6() const = 0;

    /// @brief Returns the network broadcast v6
    /// @param network Network
    virtual void broadcastV6(nlohmann::json& network) const = 0;

    /// @brief Returns the network gateway
    /// @param network Network
    virtual void gateway(nlohmann::json& network) const = 0;

    /// @brief Returns the network metrics
    /// @param network Network
    virtual void metrics(nlohmann::json& network) const = 0;

    /// @brief Returns the network metrics v6
    /// @param network Network
    virtual void metricsV6(nlohmann::json& network) const = 0;

    /// @brief Returns the network dhcp
    /// @param network Network
    virtual void dhcp(nlohmann::json& network) const = 0;

    /// @brief Returns the network mtu
    /// @param network Network
    virtual void mtu(nlohmann::json& network) const = 0;

    /// @brief Returns the network stats
    /// @return Network stats
    virtual LinkStats stats() const = 0;

    /// @brief Returns the network type
    /// @param network Network
    virtual void type(nlohmann::json& network) const = 0;

    /// @brief Returns the network state
    /// @param network Network
    virtual void state(nlohmann::json& network) const = 0;

    /// @brief Returns the network MAC
    /// @param network Network
    virtual void MAC(nlohmann::json& network) const = 0;
};
