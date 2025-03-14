#pragma once

#include "gmock/gmock.h"
#include "gtest/gtest.h"
#include <filesystem>
#include <nlohmann/json.hpp>

class MockJsonIO
{
public:
    MOCK_METHOD(nlohmann::json, readJson, (const std::filesystem::path&), ());
};
