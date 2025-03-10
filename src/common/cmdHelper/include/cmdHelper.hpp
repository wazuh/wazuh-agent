#pragma once

#include <string>

namespace Utils
{
    std::string Exec(const std::string& cmd, const size_t bufferSize = 128);
} // namespace Utils
