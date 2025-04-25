#include <sca_utils.hpp>

#include <stringHelper.hpp>

#include <optional>
#include <pcre2.h>
#include <sstream>

namespace
{
    std::pair<bool, std::string> Pcre2Match(const std::string& content, const std::string& pattern)
    {
        int error_code = 0;
        PCRE2_SIZE error_offset = 0;
        pcre2_code* re = nullptr;
        std::string matched;

        const auto pattern_ptr =
            reinterpret_cast<PCRE2_SPTR8>(pattern.c_str()); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)

        re = pcre2_compile(pattern_ptr, PCRE2_ZERO_TERMINATED, 0, &error_code, &error_offset, nullptr);

        if (re == nullptr)
        {
            return std::make_pair(false, "");
        }

        pcre2_match_data* match_data = pcre2_match_data_create_from_pattern(re, nullptr);

        const auto content_ptr =
            reinterpret_cast<PCRE2_SPTR8>(content.c_str()); // NOLINT(cppcoreguidelines-pro-type-reinterpret-cast)

        const int rc = pcre2_match(re, content_ptr, content.size(), 0, 0, match_data, nullptr);

        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        if (rc >= 0)
        {
            PCRE2_SIZE* ovector = pcre2_get_ovector_pointer(match_data);

            // rc is the number of matches found; rc >= 2 means full match + at least one capture
            if (rc >= 2)
            {
                PCRE2_SIZE start = ovector[2]; // first capture group
                PCRE2_SIZE end = ovector[3];
                matched = content.substr(start, end - start);
            }
            else
            {
                PCRE2_SIZE start = ovector[0]; // full match fallback
                PCRE2_SIZE end = ovector[1];
                matched = content.substr(start, end - start);
            }
        }
        // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        pcre2_match_data_free(match_data);
        pcre2_code_free(re);

        return std::make_pair(rc >= 0, matched);
    }

    bool
    EvaluateNumericRegexComparison(const std::string& content, const std::string& expr, sca::RegexEngineType engine)
    {
        std::istringstream stream(expr);
        std::string pattern, compare_word, op_str, expected_value_str;
        std::pair<bool, std::string> match_result {false, ""};

        if (!(stream >> pattern >> compare_word >> op_str >> expected_value_str))
        {
            return false;
        }

        if (compare_word != "compare")
        {
            return false;
        }

        const int expected_value = std::stoi(expected_value_str);

        if (engine == sca::RegexEngineType::PCRE2)
        {
            match_result = Pcre2Match(content, pattern);
        }

        if (!match_result.first)
        {
            return false;
        }

        const int actual_value = std::stoi(match_result.second);

        if (op_str == "<")
        {
            return actual_value < expected_value;
        }
        if (op_str == "<=")
        {
            return actual_value <= expected_value;
        }
        if (op_str == "==")
        {
            return actual_value == expected_value;
        }
        if (op_str == "!=")
        {
            return actual_value != expected_value;
        }
        if (op_str == ">=")
        {
            return actual_value >= expected_value;
        }
        if (op_str == ">")
        {
            return actual_value > expected_value;
        }

        return false;
    }

    bool EvaluateMinterm(const std::string& minterm, const std::string& content, sca::RegexEngineType engine)
    {
        if (minterm.starts_with("r:"))
        {
            const auto pattern = minterm.substr(2);
            if (engine == sca::RegexEngineType::PCRE2)
            {
                return Pcre2Match(content, pattern).first;
            }
        }
        else if (minterm.starts_with("n:"))
        {
            const auto expression = minterm.substr(2);
            return EvaluateNumericRegexComparison(content, expression, engine);
        }
        else
        {
            return content == minterm;
        }

        return false;
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

    bool PatternMatches(const std::string& content, const std::string& pattern, RegexEngineType engine)
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
            const bool matchResult = EvaluateMinterm(minterm, content, engine);

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
