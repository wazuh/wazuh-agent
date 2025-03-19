#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class SysInfoPackagesLinuxHelperTest : public ::testing::Test
{
protected:
    SysInfoPackagesLinuxHelperTest() = default;
    virtual ~SysInfoPackagesLinuxHelperTest() = default;

    void SetUp() override;
    void TearDown() override;
};
