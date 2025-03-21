#pragma once

#include "MockJsonIO.hpp"
#include "packagesNPM.hpp"
#include "gtest/gtest.h"
#include <mock_filesystem_utils.hpp>
#include <mock_filesystem_wrapper.hpp>

class NPMTest : public ::testing::Test
{
protected:
    std::unique_ptr<MockFileSystemUtils> mockFileSystemUtils;
    std::unique_ptr<MockFileSystemWrapper> fileSystemWrapper;
    // Raw pointer to the mock object (Workaround for unique pointer usage)
    MockFileSystemUtils* rawMockFileSystemUtils;
    MockFileSystemWrapper* rawFileSystemWrapper;
    std::unique_ptr<NPM<MockJsonIO>> npm;

    void SetUp() override
    {
        mockFileSystemUtils = std::make_unique<MockFileSystemUtils>();
        fileSystemWrapper = std::make_unique<MockFileSystemWrapper>();
        rawFileSystemWrapper = fileSystemWrapper.get();
        rawMockFileSystemUtils = mockFileSystemUtils.get();
        npm = std::make_unique<NPM<MockJsonIO>>(std::move(mockFileSystemUtils), std::move(fileSystemWrapper));
    }

    void TearDown() override
    {
        npm.reset();
    }
};
