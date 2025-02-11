/*
 * Wazuh shared modules utils
 * Copyright (C) 2015, Wazuh Inc.
 * March 16, 2021.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "byteArrayHelper_test.h"
#include "byteArrayHelper.h"

void ByteArrayHelperTest::SetUp() {};

void ByteArrayHelperTest::TearDown() {};

constexpr uint8_t BUFFER_BE[] = {0x12, 0x34, 0x56, 0x78};
constexpr uint8_t BUFFER_LE[] = {0x78, 0x56, 0x34, 0x12};
constexpr int32_t RESULT {305419896};

TEST_F(ByteArrayHelperTest, toInt32BE)
{
    EXPECT_EQ(RESULT, Utils::toInt32BE(BUFFER_BE));
}

TEST_F(ByteArrayHelperTest, toInt32LE)
{
    EXPECT_EQ(RESULT, Utils::toInt32LE(BUFFER_LE));
}
