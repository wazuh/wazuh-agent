#pragma once

#include <mock_file_io_utils.hpp>
#include <mock_filesystem_utils.hpp>
#include <mock_filesystem_wrapper.hpp>

#include "packagesPYPI.hpp"
#include "gtest/gtest.h"

class PYPITest : public ::testing::Test
{
protected:
    std::unique_ptr<MockFileSystemUtils> mockFileSystemUtils;
    std::unique_ptr<MockFileSystemWrapper> fileSystemWrapper;
    std::unique_ptr<MockFileIOUtils> mockedFileIOUtils;
    // Raw pointer to the mock object (Workaround for unique pointer usage)
    MockFileSystemUtils* rawMockFileSystemUtils;
    MockFileSystemWrapper* rawFileSystemWrapper;
    MockFileIOUtils* rawMockedFileIOUtils;
    std::unique_ptr<PYPI> pypi;

    void SetUp() override
    {
        mockFileSystemUtils = std::make_unique<MockFileSystemUtils>();
        fileSystemWrapper = std::make_unique<MockFileSystemWrapper>();
        mockedFileIOUtils = std::make_unique<MockFileIOUtils>();
        rawFileSystemWrapper = fileSystemWrapper.get();
        rawMockFileSystemUtils = mockFileSystemUtils.get();
        rawMockedFileIOUtils = mockedFileIOUtils.get();
        pypi = std::make_unique<PYPI>(
            std::move(mockFileSystemUtils), std::move(fileSystemWrapper), std::move(mockedFileIOUtils));
    }

    void TearDown() override
    {
        pypi.reset();
    }
};
