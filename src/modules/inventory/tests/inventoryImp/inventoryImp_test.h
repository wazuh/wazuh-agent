/*
 * Wazuh InventoryImp
 * Copyright (C) 2015, Wazuh Inc.
 * November 9, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */
#ifndef _INVENTORY_IMP_TEST_H
#define _INVENTORY_IMP_TEST_H
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

#endif //_INVENTORY_IMP_TEST_H