/*
 * Wazuh SyscollectorImp
 * Copyright (C) 2015, Wazuh Inc.
 * July 10, 2024.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */
#ifndef _QUEUE_TEST_H
#define _QUEUE_TEST_H
#include "gmock/gmock.h"
#include "gtest/gtest.h"

class QueueTest : public ::testing::Test
{
protected:
    QueueTest() = default;
    virtual ~QueueTest() = default;

    void SetUp() override;
    void TearDown() override;
};

class JsonTest : public ::testing::Test
{
protected:
    JsonTest() = default;
    virtual ~JsonTest() = default;

    void SetUp() override {};
    void TearDown() override {};
};

#endif //_QUEUE_TEST_H
