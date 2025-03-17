#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class PKGWrapperTest : public ::testing::Test
{
protected:
    PKGWrapperTest() = default;
    virtual ~PKGWrapperTest() = default;

    void SetUp() override;
    void TearDown() override;
};
