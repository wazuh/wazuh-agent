#pragma once

#include "packages/packageLinuxDataRetriever.h"
#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <mock_filesystem_wrapper.hpp>

class SysInfoPackagesLinuxParserRPMTest : public ::testing::Test
{
protected:
    SysInfoPackagesLinuxParserRPMTest() = default;
    virtual ~SysInfoPackagesLinuxParserRPMTest() = default;

    std::unique_ptr<MockFileSystemWrapper> fsw;

    void SetUp() override
    {
        fsw = std::make_unique<MockFileSystemWrapper>();
    }

    void TearDown() override
    {
        fsw.reset();
    }
};
