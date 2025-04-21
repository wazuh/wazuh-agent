#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace sca
{
    using PolicyVariables = std::unordered_map<std::string, std::vector<std::string>>;

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

    std::optional<std::string> GetPattern(const std::string& rule);
} // namespace sca
