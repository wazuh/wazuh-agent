#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class SysInfoHardwareWrapperX86_64MacTest : public ::testing::Test
{
protected:
    SysInfoHardwareWrapperX86_64MacTest() = default;
    virtual ~SysInfoHardwareWrapperX86_64MacTest() = default;

    void SetUp() override;
    void TearDown() override;
};
