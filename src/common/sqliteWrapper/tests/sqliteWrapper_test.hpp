#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class SQLiteTest : public ::testing::Test
{

protected:
    SQLiteTest() = default;
    virtual ~SQLiteTest() = default;

    void SetUp() override;
    void TearDown() override;
};
