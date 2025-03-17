#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class SysInfoParsersTest : public ::testing::Test
{

protected:
    SysInfoParsersTest() = default;
    virtual ~SysInfoParsersTest() = default;

    void SetUp() override;
    void TearDown() override;
};
