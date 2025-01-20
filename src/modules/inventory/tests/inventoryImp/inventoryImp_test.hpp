#pragma once
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class InventoryImpTest : public ::testing::Test
{
protected:
    InventoryImpTest() = default;
    virtual ~InventoryImpTest() = default;

    void SetUp() override;
    void TearDown() override;
};
