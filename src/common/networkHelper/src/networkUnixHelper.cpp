#include "networkUnixHelper.hpp"

#include "stringHelper.h"
#include <ifaddrs.h>
#include <map>
#include <memory>
#include <net/if.h>
#include <string>
#include <system_error>

namespace Utils
{
    void getNetworks(std::unique_ptr<ifaddrs, IfAddressSmartDeleter>& interfacesAddress,
                     std::map<std::string, std::vector<ifaddrs*>>& networkInterfaces)
    {
        struct ifaddrs* ifaddr {nullptr};
        const auto ret {getifaddrs(&ifaddr)};

        if (ret != -1)
        {
            interfacesAddress.reset(ifaddr);

            for (auto ifa = ifaddr; ifa; ifa = ifa->ifa_next)
            {
                if (!(ifa->ifa_flags & IFF_LOOPBACK) && ifa->ifa_name)
                {
                    networkInterfaces[substrOnFirstOccurrence(ifa->ifa_name, ":")].push_back(ifa);
                }
            }
        }
        else
        {
            throw std::system_error {ret, std::system_category(), "Error reading networks"};
        }
    }
} // namespace Utils
