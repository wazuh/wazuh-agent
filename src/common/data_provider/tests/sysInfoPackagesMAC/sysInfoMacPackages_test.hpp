#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class SysInfoMacPackagesTest : public ::testing::Test
{

protected:
    SysInfoMacPackagesTest() = default;
    virtual ~SysInfoMacPackagesTest() = default;

    void SetUp() override;
    void TearDown() override;
};
