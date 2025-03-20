#pragma once

#include <mock_file_io_utils.hpp>
#include <mock_filesystem_utils.hpp>
#include <mock_filesystem_wrapper.hpp>

#include "packagesPYPI.hpp"
#include "gtest/gtest.h"

class PYPITest : public ::testing::Test
{
protected:
    std::shared_ptr<MockFileSystemUtils> mockFileSystemUtils;
    std::shared_ptr<MockFileSystemWrapper> fileSystemWrapper;
    std::unique_ptr<PYPI<MockFileIOUtils>> pypi;

    void SetUp() override
    {
        mockFileSystemUtils = std::make_shared<MockFileSystemUtils>();
        fileSystemWrapper = std::make_shared<MockFileSystemWrapper>();
        pypi = std::make_unique<PYPI<MockFileIOUtils>>(mockFileSystemUtils, fileSystemWrapper);
    }

    void TearDown() override
    {
        pypi.reset();
    }
};
