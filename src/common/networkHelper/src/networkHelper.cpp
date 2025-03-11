#include "networkHelper.hpp"

#include <algorithm>
#include <arpa/inet.h>
#include <map>
#include <string>

namespace Utils
{
    std::string getNetworkTypeStringCode(const int value,
                                         const std::map<std::pair<int, int>, std::string>& interfaceTypeData)
    {
        std::string retVal;

        const auto it {std::find_if(interfaceTypeData.begin(),
                                    interfaceTypeData.end(),
                                    [value](const std::pair<std::pair<int, int>, std::string>& paramValue)
                                    { return paramValue.first.first >= value && paramValue.first.second <= value; })};

        if (interfaceTypeData.end() != it)
        {
            retVal = it->second;
        }

        return retVal;
    }

    std::string getBroadcast(const std::string& ipAddress, const std::string& netmask)
    {
        struct in_addr host
        {
        };

        struct in_addr mask
        {
        };

        struct in_addr broadcast
        {
        };

        std::string broadcastAddr;

        if (inet_pton(AF_INET, ipAddress.c_str(), &host) == 1 && inet_pton(AF_INET, netmask.c_str(), &mask) == 1)
        {

            broadcast.s_addr = host.s_addr | ~mask.s_addr;
            broadcastAddr = IAddressToBinary(AF_INET, &broadcast);
        }

        return broadcastAddr;
    }
} // namespace Utils
