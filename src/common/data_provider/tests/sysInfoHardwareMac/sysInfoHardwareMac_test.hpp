#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class SysInfoHardwareMacTest : public ::testing::Test
{
protected:
    SysInfoHardwareMacTest() = default;
    virtual ~SysInfoHardwareMacTest() = default;

    void SetUp() override;
    void TearDown() override;
};
