#pragma once

#include <gmock/gmock.h>
#include <memory>
#include <string>

#include <imodule.hpp>
#include <imoduleManager.hpp>

class MockModuleManager : public IModuleManager
{
public:
    MOCK_METHOD(void, AddModules, (), (override));
    MOCK_METHOD(std::shared_ptr<IModule>, GetModule, (const std::string& name), (override));
    MOCK_METHOD(void, Start, (), (override));
    MOCK_METHOD(void, Setup, (), (override));
    MOCK_METHOD(void, Stop, (), (override));
};
