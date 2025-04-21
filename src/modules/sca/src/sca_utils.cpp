#include <sca_utils.hpp>

namespace sca
{
    std::optional<std::pair<int, std::string>> ParseRuleType(const std::string& input)
    {
        const auto delimiter_pos = input.find(':');

        if (delimiter_pos == std::string::npos)
        {
            return std::nullopt;
        }

        auto key = input.substr(0, delimiter_pos);
        const auto value = input.substr(delimiter_pos + 1);

        if (!key.empty() && key.front() == '!')
        {
            key.erase(0, 1);
        }

        static const std::map<std::string, int> type_map = {{"f", WM_SCA_TYPE_FILE},
                                                            {"r", WM_SCA_TYPE_REGISTRY},
                                                            {"p", WM_SCA_TYPE_PROCESS},
                                                            {"d", WM_SCA_TYPE_DIR},
                                                            {"c", WM_SCA_TYPE_COMMAND}};

        const auto it = type_map.find(key);

        if (it == type_map.end())
        {
            return std::nullopt;
        }

        return std::make_pair(it->second, value);
    }

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
