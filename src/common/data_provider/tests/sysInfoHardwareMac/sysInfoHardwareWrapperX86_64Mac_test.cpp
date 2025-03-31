#include "sysInfoHardwareWrapperX86_64Mac_test.hpp"
#include "hardware/hardwareWrapperImplMac.h"
#include "osPrimitivesInterfaceMac.h"
#include "osPrimitives_mock.hpp"

void SysInfoHardwareWrapperX86_64MacTest::SetUp() {};

void SysInfoHardwareWrapperX86_64MacTest::TearDown() {};

using ::testing::_; // NOLINT(bugprone-reserved-identifier)
using ::testing::Return;

// NOLINTBEGIN(cppcoreguidelines-avoid-magic-numbers)
TEST_F(SysInfoHardwareWrapperX86_64MacTest, Test_CpuMhz_WithCpuFrequency_Succeed)
{
    auto wrapper {std::make_shared<OSHardwareWrapperMac<OsPrimitivesMacMock>>()};
    EXPECT_CALL(*wrapper, sysctlbyname("hw.cpufrequency", _, _, _, _))
        .WillOnce(
            [](const char* name, void* oldp, size_t* oldlenp, void* newp, size_t newlen)
            {
                (void)name;
                (void)oldlenp;
                (void)newp;
                (void)newlen;
                *static_cast<uint64_t*>(oldp) = 3280896;
                return 0;
            });
    int ret = 0;
    EXPECT_NO_THROW(ret = wrapper->cpuMhz());
    EXPECT_EQ(ret, 3280896 / 1000000);
}

TEST_F(SysInfoHardwareWrapperX86_64MacTest, Test_CpuMhz_WithoutCpuFrequency_Failed_Sysctlbyname)
{
    auto wrapper {std::make_shared<OSHardwareWrapperMac<OsPrimitivesMacMock>>()};
    EXPECT_CALL(*wrapper, sysctlbyname("hw.cpufrequency", _, _, _, _))
        .WillOnce(
            [](const char* name, void* oldp, size_t* oldlenp, void* newp, size_t newlen)
            {
                (void)name;
                (void)oldlenp;
                (void)newp;
                (void)newlen;
                *static_cast<uint64_t*>(oldp) = 0;
                return -1;
            });

    EXPECT_THROW(wrapper->cpuMhz(), std::system_error);
}

// NOLINTEND(cppcoreguidelines-avoid-magic-numbers)
