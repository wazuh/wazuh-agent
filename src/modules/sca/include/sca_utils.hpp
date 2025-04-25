#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace sca
{
    /// @brief Types of supported regex engines.
    enum class RegexEngineType
    {
        PCRE2,
        Invalid
    };

    /// @brief Types of supported rules.
    enum WM_SCA_TYPE
    {
        WM_SCA_TYPE_FILE,
        WM_SCA_TYPE_REGISTRY,
        WM_SCA_TYPE_PROCESS,
        WM_SCA_TYPE_DIR,
        WM_SCA_TYPE_COMMAND
    };

    /// @brief Parses the rule type from the input string.
    /// @param input The input string to parse.
    /// @return An optional pair containing the rule type and its string representation if successful.
    std::optional<std::pair<int, std::string>> ParseRuleType(const std::string& input);

    /// @brief Resolves paths in the rule string.
    /// @param ruleWithReplacedVariables The rule string with variables replaced.
    /// @return A vector of resolved paths.
    std::vector<std::filesystem::path> ResolvedPaths(const std::string& ruleWithReplacedVariables);

    /// @brief Retrieves the pattern from the rule string.
    /// @param rule The rule string to extract the pattern from.
    /// @return An optional string containing the extracted pattern if successful.
    /// @details The pattern to be returned is everything to the right of the first " -> "
    std::optional<std::string> GetPattern(const std::string& rule);

    /// @brief Checks if the content matches the given pattern using the specified regex engine.
    /// @param content The content to check against the pattern.
    /// @param pattern The pattern to match.
    /// @param engine The regex engine to use for matching.
    /// @return True if the content matches the pattern, false otherwise.
    bool PatternMatches(const std::string& content,
                        const std::string& pattern,
                        RegexEngineType engine = RegexEngineType::PCRE2);
} // namespace sca
