#include <sca_utils.hpp>

namespace
{
    // Placeholder for actual PCRE2 matching function
    bool Pcre2Match(const std::string& content, const std::string& pattern)
    {
        // Implement the actual PCRE2 matching logic here
        return content.find(pattern) != std::string::npos;
    }
} // namespace

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

    bool PatternMatches(const std::string& content, const std::string& pattern)
    {
        if (content.empty())
        {
            return false;
        }

        // Split the pattern into individual conditions (minterms)
        constexpr std::string_view delimiter = " && ";

        size_t start = 0;

        // Loop over each minterm (subpattern) in the compound pattern
        while (start < pattern.size())
        {
            // Find the next delimiter and extract the substring for this minterm
            const auto end = pattern.find(delimiter, start);
            auto minterm = pattern.substr(start, end - start);

            // Advance the start position for the next iteration
            start = (end == std::string::npos) ? end : end + delimiter.length();

            // Check if the minterm is negated
            bool negated = false;
            if (!minterm.empty() && minterm[0] == '!')
            {
                negated = true;
                minterm.erase(0, 1); // Remove the '!' for pattern matching
            }

            // Match the current minterm against the content
            const auto matchResult = Pcre2Match(content, minterm);

            // If negated, invert the match result
            const auto mintermResult = negated ^ matchResult;

            // If any minterm fails it's over
            if (!mintermResult)
            {
                return false;
            }
        }

        return true;
    }

} // namespace sca
