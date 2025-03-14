#pragma once

#include "gtest/gtest.h"

class RegistryUtilsTest : public ::testing::Test
{
protected:
    RegistryUtilsTest() = default;
    virtual ~RegistryUtilsTest() = default;

    void SetUp() override;
    void TearDown() override;
};
