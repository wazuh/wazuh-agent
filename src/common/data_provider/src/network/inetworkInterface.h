#pragma once

#include <nlohmann/json.hpp>

/// @brief Interface for network data retrievers
class IOSNetwork
{
public:
    /// @brief Default destructor
    virtual ~IOSNetwork() = default;

    /// @brief Fills the network information
    /// @param network network information
    virtual void buildNetworkData(nlohmann::json& network) = 0;
};

/// @brief Link statistics
struct LinkStats
{
    unsigned int rxPackets; /* total packets received */
    unsigned int txPackets; /* total packets transmitted */
    int64_t rxBytes;        /* total bytes received */
    int64_t txBytes;        /* total bytes transmitted */
    unsigned int rxErrors;  /* bad packets received */
    unsigned int txErrors;  /* packet transmit problems */
    unsigned int rxDropped; /* no space in linux buffers */
    unsigned int txDropped; /* no space available in linux */
};
