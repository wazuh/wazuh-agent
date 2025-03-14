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

#ifndef _NPMTEST_HPP
#define _NPMTEST_HPP

#include "gtest/gtest.h"
#include "../../../file_helper/filesystem/tests/mocks/mock_filesystem_utils.hpp"
#include "../../../file_helper/filesystem/tests/mocks/mock_filesystem_wrapper.hpp"
#include "MockJsonIO.hpp"
#include "packagesNPM.hpp"

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

#endif // _NPMTEST_HPP
