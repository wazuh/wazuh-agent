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
    std::unique_ptr<PYPI<MockFileSystemWrapper, MockFileIOUtils>> pypi;

    void SetUp() override
    {
        mockFileSystemUtils = std::make_shared<MockFileSystemUtils>();
        pypi = std::make_unique<PYPI<MockFileSystemWrapper, MockFileIOUtils>>(mockFileSystemUtils);
    }

    void TearDown() override
    {
        pypi.reset();
    }
};
