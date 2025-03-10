#pragma once

#include "gtest/gtest.h"

class HashHelperTest : public ::testing::Test
{
protected:
    HashHelperTest() = default;
    virtual ~HashHelperTest() = default;

    void SetUp() override;
    void TearDown() override;
};
