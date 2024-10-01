#pragma once
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class InvNormalizerTest : public ::testing::Test
{
    protected:

        InvNormalizerTest() = default;
        virtual ~InvNormalizerTest() = default;

        void SetUp() override;
        void TearDown() override;
};
