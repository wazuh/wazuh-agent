#pragma once

#include "iportInterface.h"

/// @brief Fields for Linux ports
enum LinuxPortsFieldsData
{
    ENTRY,
    LOCAL_ADDRESS,
    REMOTE_ADDRESS,
    STATE,
    QUEUE,
    TIMER_ACTIVE,
    RETRANSMITION,
    UID,
    TIMEOUT,
    INODE,
    SIZE_LINUX_PORT_FIELDS
};

/// @brief Interface for Linux ports
class IPortWrapper
{
public:
    /// @brief Default destructor
    virtual ~IPortWrapper() = default;

    /// @brief Fills the protocol
    /// @param port port
    virtual void protocol(nlohmann::json& port) const = 0;

    /// @brief Fills the local ip
    /// @param port port
    virtual void localIp(nlohmann::json& port) const = 0;

    /// @brief Fills the local port
    /// @param port port
    virtual void localPort(nlohmann::json& port) const = 0;

    /// @brief Fills the remote ip
    /// @param port port
    virtual void remoteIP(nlohmann::json& port) const = 0;

    /// @brief Fills the remote port
    /// @param port port
    virtual void remotePort(nlohmann::json& port) const = 0;

    /// @brief Fills the tx queue
    /// @param port port
    virtual void txQueue(nlohmann::json& port) const = 0;

    /// @brief Fills the rx queue
    /// @param port port
    virtual void rxQueue(nlohmann::json& port) const = 0;

    /// @brief Fills the inode
    /// @param port port
    virtual void inode(nlohmann::json& port) const = 0;

    /// @brief Fills the state
    /// @param port port
    virtual void state(nlohmann::json& port) const = 0;

    /// @brief Fills the pid
    /// @param port port
    virtual void pid(nlohmann::json& port) const = 0;

    /// @brief Fills the process name
    /// @param port port
    virtual void processName(nlohmann::json& port) const = 0;
};
