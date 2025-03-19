#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class SysInfoPortTest : public ::testing::Test
{

protected:
    SysInfoPortTest() = default;
    virtual ~SysInfoPortTest() = default;

    void SetUp() override;
    void TearDown() override;
};
