#pragma once

#include <gtest/gtest.h>

class CmdUtilsTest : public ::testing::Test
{
protected:
    CmdUtilsTest() = default;
    virtual ~CmdUtilsTest() = default;

    void SetUp() override;
    void TearDown() override;
};
