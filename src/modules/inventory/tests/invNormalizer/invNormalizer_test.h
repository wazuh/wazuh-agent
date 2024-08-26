/*
 * Wazuh InventoryNormalizer
 * Copyright (C) 2015, Wazuh Inc.
 * January 12, 2021.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */
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
