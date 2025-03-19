#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class SysInfoWinTest : public ::testing::Test
{
protected:
    SysInfoWinTest() = default;
    virtual ~SysInfoWinTest() = default;

    void SetUp() override;
    void TearDown() override;
};
