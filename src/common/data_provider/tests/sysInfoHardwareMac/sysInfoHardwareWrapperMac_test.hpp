#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class SysInfoHardwareWrapperMacTest : public ::testing::Test
{
protected:
    SysInfoHardwareWrapperMacTest() = default;
    virtual ~SysInfoHardwareWrapperMacTest() = default;

    void SetUp() override;
    void TearDown() override;
};
