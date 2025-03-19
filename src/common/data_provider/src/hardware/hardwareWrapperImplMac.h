#pragma once

#include "defer.hpp"
#include "hardwareWrapperInterface.h"
#include "osPrimitivesInterfaceMac.h"
#include "sharedDefs.h"
#include "stringHelper.hpp"
#include "sysInfo.hpp"
#include "utilsWrapper.hpp"
#include <sys/sysctl.h>

/// @brief Get the mhz of the cpu
/// @param osPrimitives OS primitives
/// @return mhz
int getMhz(IOsPrimitivesMac* osPrimitives = nullptr);

/// @brief MacOS hardware data wrapper
template<class TOsPrimitivesMac>
class OSHardwareWrapperMac final
    : public IOSHardwareWrapper
    , public TOsPrimitivesMac
{
public:
    /// @brief Default destructor
    virtual ~OSHardwareWrapperMac() = default;

    /// @copydoc IOSHardwareWrapper::boardSerial
    std::string boardSerial() const override
    {
        std::string ret {EMPTY_VALUE};
        const auto rawData {UtilsWrapper::exec("system_profiler SPHardwareDataType | grep Serial")};

        if (!rawData.empty())
        {
            ret = Utils::Trim(rawData.substr(rawData.find(":")), " :\t\r\n");
        }

        return ret;
    }

    /// @copydoc IOSHardwareWrapper::cpuName
    std::string cpuName() const override
    {
        size_t len {0};
        auto ret {this->sysctlbyname("machdep.cpu.brand_string", nullptr, &len, nullptr, 0)};

        if (ret)
        {
            throw std::system_error {ret, std::system_category(), "Error getting cpu name size."};
        }

        const auto spBuff {std::make_unique<char[]>(len + 1)};

        if (!spBuff)
        {
            throw std::runtime_error {"Error allocating memory to read the cpu name."};
        }

        ret = this->sysctlbyname("machdep.cpu.brand_string", spBuff.get(), &len, nullptr, 0);

        if (ret)
        {
            throw std::system_error {ret, std::system_category(), "Error getting cpu name"};
        }

        spBuff.get()[len] = 0;
        return std::string {reinterpret_cast<const char*>(spBuff.get())};
    }

    /// @copydoc IOSHardwareWrapper::cpuCores
    int cpuCores() const override
    {
        int cores {0};
        size_t len {sizeof(cores)};
        const std::vector<int> mib {CTL_HW, HW_NCPU};
        const auto ret {
            this->sysctl(const_cast<int*>(mib.data()), static_cast<u_int>(mib.size()), &cores, &len, nullptr, 0)};

        if (ret)
        {
            throw std::system_error {ret, std::system_category(), "Error reading cpu cores number."};
        }

        return cores;
    }

    /// @copydoc IOSHardwareWrapper::cpuMhz
    int cpuMhz() override
    {
        return getMhz(static_cast<IOsPrimitivesMac*>(this));
    }

    /// @copydoc IOSHardwareWrapper::ramTotal
    uint64_t ramTotal() const override
    {
        uint64_t ramTotal {0};
        size_t len {sizeof(ramTotal)};
        auto ret {this->sysctlbyname("hw.memsize", &ramTotal, &len, nullptr, 0)};

        if (ret)
        {
            throw std::system_error {ret, std::system_category(), "Error reading total RAM."};
        }

        return ramTotal / KByte;
    }

    /// @copydoc IOSHardwareWrapper::ramFree
    uint64_t ramFree() const override
    {
        u_int pageSize {0};
        size_t len {sizeof(pageSize)};
        auto ret {this->sysctlbyname("vm.pagesize", &pageSize, &len, nullptr, 0)};

        if (ret)
        {
            throw std::system_error {ret, std::system_category(), "Error reading page size."};
        }

        uint64_t freePages {0};
        len = sizeof(freePages);
        ret = this->sysctlbyname("vm.page_free_count", &freePages, &len, nullptr, 0);

        if (ret)
        {
            throw std::system_error {ret, std::system_category(), "Error reading pages free count."};
        }

        return (freePages * pageSize) / KByte;
    }

    /// @copydoc IOSHardwareWrapper::ramUsage
    uint64_t ramUsage() const override
    {
        uint64_t ret {0};
        const auto ramTotal {this->ramTotal()};

        if (ramTotal)
        {
            ret = 100 - (100 * ramFree() / ramTotal);
        }

        return ret;
    }
};
