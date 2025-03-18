#include "hardwareWrapperImplMac.h"

int getMhz(IOsPrimitivesMac* osPrimitives)
{
    constexpr auto MHz {1000000};
    uint64_t cpuHz {0};
    size_t len {sizeof(cpuHz)};
    int ret {osPrimitives->sysctlbyname("hw.cpufrequency", &cpuHz, &len, nullptr, 0)};

    if (ret)
    {
        throw std::system_error {ret, std::system_category(), "Error reading cpu frequency."};
    }

    return static_cast<int>(cpuHz / MHz);
}
