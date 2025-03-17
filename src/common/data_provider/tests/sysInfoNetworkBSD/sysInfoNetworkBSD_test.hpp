#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class SysInfoNetworkBSDTest : public ::testing::Test
{

protected:
    SysInfoNetworkBSDTest() = default;
    virtual ~SysInfoNetworkBSDTest() = default;

    void SetUp() override;
    void TearDown() override;
};
