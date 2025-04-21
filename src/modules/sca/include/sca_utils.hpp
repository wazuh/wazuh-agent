#pragma once

#include <filesystem>
#include <map>
#include <optional>
#include <string>
#include <vector>

namespace sca
{
    struct StringLengthGreater
    {
        bool operator()(const std::string& a, const std::string& b) const
        {
            return a.length() > b.length() || (a.length() == b.length() && a < b);
        }
    };

    using PolicyVariables = std::map<std::string, std::vector<std::string>, StringLengthGreater>;

    enum WM_SCA_TYPE
    {
        WM_SCA_TYPE_FILE,
        WM_SCA_TYPE_REGISTRY,
        WM_SCA_TYPE_PROCESS,
        WM_SCA_TYPE_DIR,
        WM_SCA_TYPE_COMMAND
    };

    std::optional<std::pair<int, std::string>> ParseRuleType(const std::string& input);

    std::string RuleWithReplacedVariables(const PolicyVariables& variables, std::string rule);

    std::vector<std::filesystem::path> ResolvedPaths(const std::string& ruleWithReplacedVariables);

    std::optional<std::string> GetPattern(const std::string& rule);
} // namespace sca
