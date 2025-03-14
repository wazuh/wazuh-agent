#pragma once

#include "gtest/gtest.h"

class LinuxInfoHelperTest : public ::testing::Test
{
protected:
    LinuxInfoHelperTest() = default;
    virtual ~LinuxInfoHelperTest() = default;

    void SetUp() override;
    void TearDown() override;
};
