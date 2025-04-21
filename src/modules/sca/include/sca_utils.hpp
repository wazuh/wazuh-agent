#pragma once

#include <filesystem>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>

namespace sca
{
    using PolicyVariables = std::unordered_map<std::string, std::vector<std::string>>;

    std::string RuleWithReplacedVariables(const PolicyVariables& variables, std::string rule);

    std::optional<std::string> GetPattern(const std::string& rule);
} // namespace sca
