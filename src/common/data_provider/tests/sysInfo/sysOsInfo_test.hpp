#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class SysOsInfoTest : public ::testing::Test
{

protected:
    SysOsInfoTest() = default;
    virtual ~SysOsInfoTest() = default;

    void SetUp() override;
    void TearDown() override;
};
