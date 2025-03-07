#pragma once

#include "gtest/gtest.h"

class ByteArrayHelperTest : public ::testing::Test
{
protected:
    ByteArrayHelperTest() = default;
    virtual ~ByteArrayHelperTest() = default;

    void SetUp() override;
    void TearDown() override;
};
