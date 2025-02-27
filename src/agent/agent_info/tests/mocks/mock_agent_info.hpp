#pragma once

#include <gmock/gmock.h>

#include <iagent_info.hpp>

#include <string>
#include <vector>

class MockAgentInfo : public IAgentInfo
{
public:
    MOCK_METHOD(std::string, GetName, (), (const, override));
    MOCK_METHOD(std::string, GetKey, (), (const, override));
    MOCK_METHOD(std::string, GetUUID, (), (const, override));
    MOCK_METHOD(std::vector<std::string>, GetGroups, (), (const, override));
    MOCK_METHOD(bool, SetName, (const std::string& name), (override));
    MOCK_METHOD(bool, SetKey, (const std::string& key), (override));
    MOCK_METHOD(void, SetUUID, (const std::string& uuid), (override));
    MOCK_METHOD(void, SetGroups, (const std::vector<std::string>& groupList), (override));
    MOCK_METHOD(std::string, GetType, (), (const, override));
    MOCK_METHOD(std::string, GetVersion, (), (const, override));
    MOCK_METHOD(std::string, GetHeaderInfo, (), (const, override));
    MOCK_METHOD(std::string, GetMetadataInfo, (), (const, override));
    MOCK_METHOD(void, Save, (), (const, override));
    MOCK_METHOD(bool, SaveGroups, (), (const, override));
};
