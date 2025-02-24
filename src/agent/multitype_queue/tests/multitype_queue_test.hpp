#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class MultiTypeQueueTest : public ::testing::Test
{
protected:
    MockStorage* m_mockStorage = nullptr;
    std::unique_ptr<MockStorage> m_mockStoragePtr;

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
