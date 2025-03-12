#pragma once

#include "gtest/gtest.h"

class JsonIOTest : public ::testing::Test
{
protected:
    JsonIOTest() = default;
    virtual ~JsonIOTest() = default;

    void SetUp() override;
    void TearDown() override;
};
