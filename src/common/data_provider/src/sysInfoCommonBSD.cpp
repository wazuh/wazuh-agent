#include "network/networkBSDWrapper.h"
#include "network/networkFamilyDataAFactory.h"
#include "networkUnixHelper.hpp"
#include "stringHelper.hpp"
#include "sysInfo.hpp"
#include <sys/sysctl.h>
#include <sys/types.h>

nlohmann::json SysInfo::getNetworks() const
{
    nlohmann::json networks;

    std::unique_ptr<ifaddrs, Utils::IfAddressSmartDeleter> interfacesAddress;
    std::map<std::string, std::vector<ifaddrs*>> networkInterfaces;
    Utils::getNetworks(interfacesAddress, networkInterfaces);

    for (const auto& interface : networkInterfaces)
    {
        nlohmann::json ifaddr {};

        for (const auto addr : interface.second)
        {
            const auto networkInterfacePtr {FactoryNetworkFamilyCreator<OSPlatformType::BSDBASED>::create(
                std::make_shared<NetworkBSDInterface>(addr))};

            if (networkInterfacePtr)
            {
                networkInterfacePtr->buildNetworkData(ifaddr);
            }
        }

        networks["iface"].push_back(ifaddr);
    }

    return networks;
}
