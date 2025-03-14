/*
 * Wazuh SysInfo
 * Copyright (C) 2015, Wazuh Inc.
 * July 16, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _PYPITEST_HPP
#define _PYPITEST_HPP

#include "gtest/gtest.h"
#include "../../../file_helper/filesystem/tests/mocks/mock_filesystem_wrapper.hpp"
#include "../../../file_helper/filesystem/tests/mocks/mock_filesystem_utils.hpp"
#include "../../../file_helper/file_io/tests/mocks/mock_file_io_utils.hpp"
#include "packagesPYPI.hpp"

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

#endif // _PYPITEST_HPP
