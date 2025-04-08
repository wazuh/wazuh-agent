#pragma once

#include <gmock/gmock.h>
#include <memory>
#include <string>

#include <imoduleManager.hpp>
#include <moduleWrapper.hpp>

class MockModuleManager : public IModuleManager
{
public:
    MOCK_METHOD(void, AddModules, (), (override));
    MOCK_METHOD(std::shared_ptr<ModuleWrapper>, GetModule, (const std::string& name), (override));
    MOCK_METHOD(void, Start, (), (override));
    MOCK_METHOD(void, Setup, (), (override));
    MOCK_METHOD(void, Stop, (), (override));
};
