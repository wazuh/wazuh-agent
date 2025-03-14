#include "globHelper.hpp"

#include <string>

namespace Utils
{
    bool patternMatch(const std::string& entryName, const std::string& pattern)
    {
        auto match {true};
        // Match the glob pattern without regex
        auto patternPos {0u};

        for (auto i {0u}; i < entryName.size(); ++i)
        {
            if (patternPos < pattern.size())
            {
                // 'x' matches 'x', '?' matches any single character
                if ((entryName.at(i) == pattern.at(patternPos)) || pattern.at(patternPos) == '?')
                {
                    ++patternPos;
                }
                // '*' matches any number of characters
                else if (pattern.at(patternPos) == '*')
                {
                    // '*' matches zero characters
                    if (patternPos + 1 < pattern.size() && pattern.at(patternPos + 1) == entryName.at(i))
                    {
                        ++patternPos;
                        --i;
                    }
                    // '*' matches one or more characters
                    else if (patternPos + 1 == pattern.size())
                    {
                        break;
                    }
                }
                // No match
                else
                {
                    match = false;
                    break;
                }
            }
            else
            {
                match = false;
                break;
            }
        }

        // if the pattern is not fully matched, check if the remaining characters are '*'
        // and if so, the match is successful.
        while (match && patternPos < pattern.size())
        {
            // '*' matches zero characters
            if (pattern.at(patternPos) == '*')
            {
                ++patternPos;
            }
            // No match
            else
            {
                match = false;
            }
        }

        return match;
    }
} // namespace Utils
