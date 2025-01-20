#pragma once
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class InvNormalizerTest : public ::testing::Test
{
protected:
    InvNormalizerTest() = default;
    virtual ~InvNormalizerTest() = default;

    void SetUp() override;
    void TearDown() override;
};
