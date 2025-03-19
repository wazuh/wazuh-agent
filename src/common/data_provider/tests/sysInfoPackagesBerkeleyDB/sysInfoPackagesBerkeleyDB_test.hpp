#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class SysInfoPackagesBerkeleyDBTest : public ::testing::Test
{
protected:
    SysInfoPackagesBerkeleyDBTest() = default;
    virtual ~SysInfoPackagesBerkeleyDBTest() = default;

    void SetUp() override;
    void TearDown() override;
};
