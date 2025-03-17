#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class SysInfoTest : public ::testing::Test
{

protected:
    SysInfoTest() = default;
    virtual ~SysInfoTest() = default;

    void SetUp() override;
    void TearDown() override;
};
