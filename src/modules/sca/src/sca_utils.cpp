#include <sca_utils.hpp>

namespace sca
{
    std::string RuleWithReplacedVariables(const PolicyVariables& variables, std::string rule)
    {
        for (const auto& [var, values] : variables)
        {
            if (values.empty())
            {
                continue;
            }

            const auto& replacement = values.front();
            size_t pos = 0;

            while ((pos = rule.find(var, pos)) != std::string::npos)
            {
                rule.replace(pos, var.length(), replacement);
                pos += replacement.length();
            }
        }

        return rule;
    }

    std::vector<std::filesystem::path> ResolvedPaths(const std::string& ruleWithReplacedVariables)
    {
        std::vector<std::filesystem::path> paths;
        size_t start = 0;

        while (start < ruleWithReplacedVariables.size())
        {
            auto end = ruleWithReplacedVariables.find(',', start);

            if (end == std::string::npos)
            {
                end = ruleWithReplacedVariables.size();
            }

            auto path = ruleWithReplacedVariables.substr(start, end - start);
            path.erase(0, path.find_first_not_of(" \t"));
            path.erase(path.find_last_not_of(" \t") + 1);

            if (!path.empty())
            {
                paths.emplace_back(path);
            }

            start = end + 1;
        }

        return paths;
    }

    std::optional<std::string> GetPattern(const std::string& rule)
    {
        const std::string delimiter = " -> ";
        const auto pos = rule.find(delimiter);

        if (pos != std::string::npos)
        {
            return rule.substr(pos + delimiter.size());
        }

        return std::nullopt;
    }
} // namespace sca
