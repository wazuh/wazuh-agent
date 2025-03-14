#pragma once

#include "gtest/gtest.h"

class WindowsHelperTest : public ::testing::Test
{
protected:
    WindowsHelperTest() = default;
    virtual ~WindowsHelperTest() = default;

    void SetUp() override;
    void TearDown() override;
};
