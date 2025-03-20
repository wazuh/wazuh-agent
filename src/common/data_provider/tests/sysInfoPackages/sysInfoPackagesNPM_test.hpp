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
    std::shared_ptr<MockFileSystemWrapper> fileSystemWrapper;
    std::unique_ptr<NPM<MockJsonIO>> npm;

    void SetUp() override
    {
        mockFileSystemUtils = std::make_shared<MockFileSystemUtils>();
        fileSystemWrapper = std::make_shared<MockFileSystemWrapper>();
        npm = std::make_unique<NPM<MockJsonIO>>(mockFileSystemUtils, fileSystemWrapper);
    }

    void TearDown() override
    {
        npm.reset();
    }
};
