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
