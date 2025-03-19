#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class RpmLibTest : public ::testing::Test
{
protected:
    RpmLibTest() = default;
    virtual ~RpmLibTest() = default;

    void SetUp() override;
    void TearDown() override;
};
