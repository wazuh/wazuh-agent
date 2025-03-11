#pragma once

#include <gtest/gtest.h>

class TimeUtilsTest : public ::testing::Test
{
protected:
    TimeUtilsTest() = default;
    virtual ~TimeUtilsTest() = default;

    void SetUp() override;
    void TearDown() override;
};
