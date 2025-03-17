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
    std::unique_ptr<RPM<MockFileSystemWrapper>> rpm;

    void SetUp() override
    {
        rpm = std::make_unique<RPM<MockFileSystemWrapper>>();
    }

    void TearDown() override
    {
        rpm.reset();
    }
};
