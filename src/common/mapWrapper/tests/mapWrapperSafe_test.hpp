#pragma once

#include "gtest/gtest.h"

class MapWrapperSafeTest : public ::testing::Test
{
protected:
    MapWrapperSafeTest() = default;
    virtual ~MapWrapperSafeTest() = default;

    void SetUp() override;
    void TearDown() override;
};
