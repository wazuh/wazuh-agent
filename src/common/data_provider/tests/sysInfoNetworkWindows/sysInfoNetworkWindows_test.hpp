#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class SysInfoNetworkWindowsTest : public ::testing::Test
{
protected:
    SysInfoNetworkWindowsTest() = default;
    virtual ~SysInfoNetworkWindowsTest() = default;

    void SetUp() override;
    void TearDown() override;
};
