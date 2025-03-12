#pragma once

#include "sqliteWrapperFactory.hpp"
#include <gmock/gmock.h>
#include <string>

class MockSQLiteFactory : public SQLiteLegacy::ISQLiteFactory
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
