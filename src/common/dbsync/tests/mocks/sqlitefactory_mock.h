/*
 * Wazuh DBSYNC
 * Copyright (C) 2015, Wazuh Inc.
 * June 16, 2020.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _MOCKSQLITEFACTORY_TEST_H
#define _MOCKSQLITEFACTORY_TEST_H

#include "sqlite/sqlite_wrapper_factory.h"
#include <gmock/gmock.h>
#include <string>

class MockSQLiteFactory : public ISQLiteFactory
{
public:
    MOCK_METHOD(std::shared_ptr<SQLiteLegacy::IConnection>, createConnection, (const std::string& path), (override));
    MOCK_METHOD(std::unique_ptr<SQLiteLegacy::ITransaction>,
                createTransaction,
                (std::shared_ptr<SQLiteLegacy::IConnection> & connection),
                (override));
    MOCK_METHOD(std::unique_ptr<SQLiteLegacy::IStatement>,
                createStatement,
                (std::shared_ptr<SQLiteLegacy::IConnection> & connection, const std::string& query),
                (override));
};

#endif //_MOCKSQLITEFACTORY_TEST_H
