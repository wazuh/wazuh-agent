#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class SysInfoNetworkLinuxTest : public ::testing::Test
{

protected:
    SysInfoNetworkLinuxTest() = default;
    virtual ~SysInfoNetworkLinuxTest() = default;

    void SetUp() override;
    void TearDown() override;
};
