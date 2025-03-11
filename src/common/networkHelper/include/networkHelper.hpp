#pragma once

#include <arpa/inet.h>
#include <map>
#include <memory>
#include <netdb.h>
#include <string>

namespace Utils
{
    /// @brief Get the network type string code
    /// @param value the value to check
    /// @param interfaceTypeData the interface type data
    /// @return the network type
    std::string getNetworkTypeStringCode(const int value,
                                         const std::map<std::pair<int, int>, std::string>& interfaceTypeData);

    /// @brief Converts the address to binary
    /// @param family the family
    /// @param address the address
    /// @return the binary address
    template<class T>
    std::string IAddressToBinary(const int family, const T address)
    {
        std::string retVal;
        const auto broadcastAddrPlain {std::make_unique<char[]>(NI_MAXHOST)};

        if (inet_ntop(family, address, broadcastAddrPlain.get(), NI_MAXHOST))
        {
            retVal = broadcastAddrPlain.get();
        }

        return retVal;
    }

    /// @brief Get the broadcast address
    /// @param ipAddress the ip address
    /// @param netmask the netmask
    /// @return the broadcast address
    std::string getBroadcast(const std::string& ipAddress, const std::string& netmask);
} // namespace Utils
