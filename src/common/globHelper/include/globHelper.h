/*
 * Wazuh shared modules utils
 * Copyright (C) 2015, Wazuh Inc.
 * Agoust 11, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _GLOB_HELPER_H
#define _GLOB_HELPER_H

#include <string>

namespace Utils
{
#ifdef __GNUC__
#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wunused-function"
#endif

#ifdef _MSC_VER
#pragma warning(push)
#pragma warning(disable: 4505)
#endif

static bool patternMatch(const std::string& entryName, const std::string& pattern)
{
    std::string_view entry(entryName);
    std::string_view pat(pattern);

    // Dynamic programming approach - create a table for memoization
    // dp[i][j] is true if the first i characters of entry match the first j characters of pattern
    std::vector<std::vector<bool>> dp(entry.size() + 1, std::vector<bool>(pat.size() + 1, false));

    // Empty pattern matches empty string
    dp[0][0] = true;

    // Handle patterns like "*", "*a", "a*b*" etc. where '*' can match empty strings
    for (size_t j = 1; j <= pat.size(); ++j)
    {
        if (pat[j - 1] == '*')
        {
            dp[0][j] = dp[0][j - 1];
        }
    }

    // Fill the dp table
    for (size_t i = 1; i <= entry.size(); ++i)
    {
        for (size_t j = 1; j <= pat.size(); ++j)
        {
            if (pat[j - 1] == '*')
            {
                // '*' can match 0 or more characters
                // Either ignore '*' (dp[i][j-1]) or use '*' to match current character (dp[i-1][j])
                dp[i][j] = dp[i][j - 1] || dp[i - 1][j];
            }
            else if (pat[j - 1] == '?' || entry[i - 1] == pat[j - 1])
            {
                // '?' matches any single character, or exact character match
                dp[i][j] = dp[i - 1][j - 1];
            }
        }
    }

    return dp[entry.size()][pat.size()];
}

#ifdef __GNUC__
#pragma GCC diagnostic pop
#endif

#ifdef _MSC_VER
#pragma warning(pop)
#endif

}

#endif // _GLOB_HELPER_H
