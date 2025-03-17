#pragma once

#include "MockJsonIO.hpp"
#include "packagesNPM.hpp"
#include "gtest/gtest.h"
#include <mock_filesystem_utils.hpp>
#include <mock_filesystem_wrapper.hpp>

class NPMTest : public ::testing::Test
{
protected:
    std::shared_ptr<MockFileSystemUtils> mockFileSystemUtils;
    std::unique_ptr<NPM<MockFileSystemWrapper, MockJsonIO>> npm;

    void SetUp() override
    {
        mockFileSystemUtils = std::make_shared<MockFileSystemUtils>();
        npm = std::make_unique<NPM<MockFileSystemWrapper, MockJsonIO>>(mockFileSystemUtils);
    }

    void TearDown() override
    {
        npm.reset();
    }
};
