/*
 * Wazuh shared modules utils
 * Copyright (C) 2015, Wazuh Inc.
 * October 23, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#include "filesystemHelper_test.h"

void FilesystemUtilsTest::SetUp() {};

void FilesystemUtilsTest::TearDown() {};

TEST_F(FilesystemUtilsTest, getFileContent)
{
    const auto& inputFile{"/etc/services"};
    const auto content {Utils::getFileContent(inputFile)};
    EXPECT_FALSE(content.empty());
}

TEST_F(FilesystemUtilsTest, getFileBinaryContent)
{
    const auto binContent {Utils::getBinaryContent("/usr/bin/gcc")};
    EXPECT_FALSE(binContent.empty());
}
