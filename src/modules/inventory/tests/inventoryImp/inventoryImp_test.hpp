#pragma once
#include "gmock/gmock.h"
#include "gtest/gtest.h"

#include <inventory.hpp>

class InventoryImpTest : public ::testing::Test
{
protected:
    InventoryImpTest() = default;
    virtual ~InventoryImpTest() = default;

    void SetUp() override;
    void TearDown() override;

    Inventory m_inventory;
};
