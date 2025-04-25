#pragma once

#include <map>
#include <optional>
#include <string>
#include <vector>

namespace sca
{
    /// @brief A custom comparator for std::map that sorts keys by string length in descending order.
    struct StringLengthGreater
    {
        bool operator()(const std::string& a, const std::string& b) const
        {
            return a.length() > b.length() || (a.length() == b.length() && a < b);
        }
    };

    /// @brief A container for policy variables that will sort them by length in descending order.
    /// @details This is used to ensure that longer keys are checked first when replacing variables in rules.
    using PolicyVariables = std::map<std::string, std::vector<std::string>, StringLengthGreater>;

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

    /// @brief Replaces variables in the rule with their corresponding values from the policy variables.
    /// @param variables The policy variables to use for replacement.
    /// @param rule The rule string with variables to be replaced.
    /// @return The rule string with variables replaced by their corresponding values.
    std::string RuleWithReplacedVariables(const PolicyVariables& variables, std::string rule);

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
