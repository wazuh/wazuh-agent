#pragma once

#include "gmock/gmock.h"

#include <istorage.hpp>

#include "nlohmann/json.hpp"

#include <string>
#include <vector>

class MockStorage : public IStorage
{
public:
    MOCK_METHOD(bool, Clear, (const std::vector<std::string>& tableNames), (override));

    MOCK_METHOD(int,
                Store,
                (const nlohmann::json& message,
                 const std::string& tableName,
                 const std::string& moduleName,
                 const std::string& moduleType,
                 const std::string& metadata),
                (override));

    MOCK_METHOD(int,
                RemoveMultiple,
                (int n, const std::string& tableName, const std::string& moduleName, const std::string& moduleType),
                (override));

    MOCK_METHOD(nlohmann::json,
                RetrieveMultiple,
                (int n, const std::string& tableName, const std::string& moduleName, const std::string& moduleType),
                (override));

    MOCK_METHOD(nlohmann::json,
                RetrieveBySize,
                (size_t n, const std::string& tableName, const std::string& moduleName, const std::string& moduleType),
                (override));

    MOCK_METHOD(int,
                GetElementCount,
                (const std::string& tableName, const std::string& moduleName, const std::string& moduleType),
                (override));

    MOCK_METHOD(size_t,
                GetElementsStoredSize,
                (const std::string& tableName, const std::string& moduleName, const std::string& moduleType),
                (override));
};
