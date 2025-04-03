#pragma once

#include <filesystem>

class SCAPolicy
{
public:
    static SCAPolicy LoadFromFile([[maybe_unused]] const std::filesystem::path& path)
    {
        return {};
    }
};
