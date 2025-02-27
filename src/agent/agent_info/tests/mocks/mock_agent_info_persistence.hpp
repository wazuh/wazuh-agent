#pragma once

#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <iagent_info_persistence.hpp>

#include <string>
#include <vector>

class MockAgentInfoPersistence : public IAgentInfoPersistence
{
public:
    MOCK_METHOD(std::string, GetName, (), (const, override));
    MOCK_METHOD(std::string, GetKey, (), (const, override));
    MOCK_METHOD(std::string, GetUUID, (), (const, override));
    MOCK_METHOD(std::vector<std::string>, GetGroups, (), (const, override));
    MOCK_METHOD(bool, SetName, (const std::string& name), (override));
    MOCK_METHOD(bool, SetKey, (const std::string& key), (override));
    MOCK_METHOD(bool, SetUUID, (const std::string& uuid), (override));
    MOCK_METHOD(bool, SetGroups, (const std::vector<std::string>& groupList), (override));
    MOCK_METHOD(bool, ResetToDefault, (), (override));
};
