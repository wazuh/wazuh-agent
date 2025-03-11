#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"

class PipelineNodesTest : public ::testing::Test
{
protected:
    PipelineNodesTest() = default;
    virtual ~PipelineNodesTest() = default;

    void SetUp() override;
    void TearDown() override;
};
