#pragma once

#include "iportInterface.h"
#include "iportWrapper.h"
#include "sharedDefs.h"

/// @brief Port implementation
class PortImpl final : public IOSPort
{
private:
    std::shared_ptr<IPortWrapper> m_spPortRawData;

public:
    /// @brief Constructor
    /// @param portRawData port raw data
    explicit PortImpl(const std::shared_ptr<IPortWrapper>& portRawData)
        : m_spPortRawData(portRawData)
    {
    }

    /// @brief Destructor
    ~PortImpl() = default;

    /// @copydoc IOSPort::buildPortData
    void buildPortData(nlohmann::json& port) override
    {
        m_spPortRawData->protocol(port);
        m_spPortRawData->localIp(port);
        m_spPortRawData->localPort(port);
        m_spPortRawData->remoteIP(port);
        m_spPortRawData->remotePort(port);
        m_spPortRawData->txQueue(port);
        m_spPortRawData->rxQueue(port);
        m_spPortRawData->inode(port);
        m_spPortRawData->state(port);
        m_spPortRawData->pid(port);
        m_spPortRawData->processName(port);
    }
};
