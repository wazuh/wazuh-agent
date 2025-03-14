#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class ThreadDispatcherTest : public ::testing::Test
{
protected:
    ThreadDispatcherTest() = default;
    virtual ~ThreadDispatcherTest() = default;

    void SetUp() override;
    void TearDown() override;
};
