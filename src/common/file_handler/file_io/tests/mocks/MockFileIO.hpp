#pragma once

#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include <filesystem>

class MockFileIO
{
    public:
        MOCK_METHOD(void, readLineByLine, (const std::filesystem::path&, const std::function<bool(const std::string&)>&), ());
};
