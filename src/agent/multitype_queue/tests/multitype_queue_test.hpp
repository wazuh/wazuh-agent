#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class MultiTypeQueueTest : public ::testing::Test
{
protected:
    MultiTypeQueueTest() = default;
    virtual ~MultiTypeQueueTest() = default;

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
