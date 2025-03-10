#pragma once

#include "gtest/gtest.h"

class EncodingWindowsHelperTest : public ::testing::Test
{
protected:
    EncodingWindowsHelperTest() = default;
    virtual ~EncodingWindowsHelperTest() = default;

    void SetUp() override;
    void TearDown() override;
};
