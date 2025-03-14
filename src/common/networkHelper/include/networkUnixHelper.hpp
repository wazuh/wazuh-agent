#pragma once

#include <ifaddrs.h>
#include <map>
#include <memory>
#include <string>
#include <vector>

namespace Utils
{
    /// @brief Smart deleter for ifaddrs
    struct IfAddressSmartDeleter
    {
        void operator()(ifaddrs* address)
        {
            freeifaddrs(address);
        }
    };

    /// @brief Get the networks
    /// @param interfacesAddress the interfaces address
    /// @param networkInterfaces the network interfaces
    void getNetworks(std::unique_ptr<ifaddrs, IfAddressSmartDeleter>& interfacesAddress,
                     std::map<std::string, std::vector<ifaddrs*>>& networkInterfaces);
} // namespace Utils
