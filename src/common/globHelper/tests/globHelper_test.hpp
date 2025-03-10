#pragma once

#include "gtest/gtest.h"

class GlobHelperTest : public ::testing::Test
{
protected:
    GlobHelperTest() = default;
    virtual ~GlobHelperTest() = default;

    void SetUp() override;
    void TearDown() override;
};
