#pragma once
#include "gtest/gtest.h"
#include "gmock/gmock.h"

class InventoryImpTest : public ::testing::Test
{
    protected:

        InventoryImpTest() = default;
        virtual ~InventoryImpTest() = default;

        void SetUp() override;
        void TearDown() override;
};
