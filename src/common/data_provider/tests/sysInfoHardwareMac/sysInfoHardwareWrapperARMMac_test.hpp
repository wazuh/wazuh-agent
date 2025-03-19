#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class SysInfoHardwareWrapperARMMacTest : public ::testing::Test
{
protected:
    SysInfoHardwareWrapperARMMacTest() = default;
    virtual ~SysInfoHardwareWrapperARMMacTest() = default;

    void SetUp() override;
    void TearDown() override;
};
