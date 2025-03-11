#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class NetworkWindowsHelperTest : public ::testing::Test
{
protected:
    NetworkWindowsHelperTest() = default;
    virtual ~NetworkWindowsHelperTest() = default;

    void SetUp() override;
    void TearDown() override;
};
