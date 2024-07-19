/*
 * Wazuh URLRequest unit tests
 * Copyright (C) 2015, Wazuh Inc.
 * October 30, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef __SECURE_COMMUNICATION_TEST_HPP
#define __SECURE_COMMUNICATION_TEST_HPP

#include "gtest/gtest.h"

/**
 * @brief SecureCommunicationTest class.
 *
 */
class SecureCommunicationTest : public ::testing::Test
{
protected:
    SecureCommunicationTest() = default;
    virtual ~SecureCommunicationTest() = default;
};

#endif // __SECURE_COMMUNICATION_TEST_HPP
