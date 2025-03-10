#pragma once

#include <string>

namespace Utils
{
    /// @brief Check if a string matches a glob pattern
    /// @param entryName string to check
    /// @param pattern glob pattern to match
    /// @return true if the string matches the glob pattern
    bool patternMatch(const std::string& entryName, const std::string& pattern);
} // namespace Utils
