#include <sca_utils.hpp>

#include <logger.hpp>
#include <stringHelper.hpp>

#include <pcre2.h>

#include <map>
#include <optional>
#include <sstream>
#include <stdexcept>

namespace
{
    std::pair<bool, std::string> Pcre2Match(const std::string& content, const std::string& pattern)
    {
        int error_code = 0;
        PCRE2_SIZE error_offset = 0;

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const auto pattern_ptr = reinterpret_cast<PCRE2_SPTR8>(pattern.c_str());

        auto* re =
            pcre2_compile(pattern_ptr, PCRE2_ZERO_TERMINATED, PCRE2_MULTILINE, &error_code, &error_offset, nullptr);

        if (!re)
        {
            throw std::runtime_error("PCRE2 compilation failed");
        }

        auto* match_data = pcre2_match_data_create_from_pattern(re, nullptr);

        if (!match_data)
        {
            pcre2_code_free(re);
            throw std::runtime_error("PCRE2 match data creation failed");
        }

        // NOLINTNEXTLINE(cppcoreguidelines-pro-type-reinterpret-cast)
        const auto content_ptr = reinterpret_cast<PCRE2_SPTR8>(content.c_str());
        const auto rc = pcre2_match(re, content_ptr, content.size(), 0, 0, match_data, nullptr);

        auto pcre2CleanUp = [match_data, re]()
        {
            pcre2_match_data_free(match_data);
            pcre2_code_free(re);
        };

        if (rc == PCRE2_ERROR_NOMATCH)
        {
            // No match, but not an error
            pcre2CleanUp();
            return {false, ""};
        }
        else if (rc < 0)
        {
            // Other matching error
            pcre2CleanUp();
            throw std::runtime_error("PCRE2 match error: " + std::to_string(rc));
        }

        const auto* ovector = pcre2_get_ovector_pointer(match_data);

        if (!ovector)
        {
            pcre2CleanUp();
            throw std::runtime_error("PCRE2 ovector pointer is null");
        }

        // NOLINTBEGIN(cppcoreguidelines-pro-bounds-pointer-arithmetic)
        const auto match = rc >= 2 ? content.substr(ovector[2], ovector[3] - ovector[2])
                                   : content.substr(ovector[0], ovector[1] - ovector[0]);
        // NOLINTEND(cppcoreguidelines-pro-bounds-pointer-arithmetic)

        pcre2CleanUp();

        return {true, match};
    }

    bool
    EvaluateNumericRegexComparison(const std::string& content, const std::string& expr, sca::RegexEngineType engine)
    {
        std::istringstream stream(expr);
        std::string pattern, compare_word, op_str, expected_value_str;
        std::pair<bool, std::string> match_result {false, ""};

        if (!(stream >> pattern >> compare_word >> op_str >> expected_value_str))
        {
            throw std::runtime_error("Invalid expression format");
        }

        if (compare_word != "compare")
        {
            throw std::runtime_error("Expected 'compare' keyword in numeric comparison");
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

        throw std::runtime_error("Invalid operator in numeric comparison");
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
    std::string_view CheckResultToString(const CheckResult result)
    {
        switch (result)
        {
            case CheckResult::Passed: return "Passed";
            case CheckResult::Failed: return "Failed";
            case CheckResult::NotApplicable: return "Not applicable";
            case CheckResult::NotRun: return "Not run";
            default: return "Unknown";
        }
    }

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

    std::optional<bool> PatternMatches(const std::string& content, const std::string& pattern, RegexEngineType engine)
    {
        try
        {
            if (content.empty())
            {
                return false;
            }

            // Split the pattern into individual conditions (minterms)
            constexpr std::string_view delimiter = " && ";
            std::vector<std::pair<bool, std::string>> minterms; // (negated, pattern)

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

                minterms.emplace_back(negated, minterm);
            }

            // Special case: if there's only one minterm and it's negated
            if (minterms.size() == 1 && minterms[0].first)
            {
                const auto& minterm = minterms[0].second;
                std::istringstream stream(content);
                std::string line;
                while (std::getline(stream, line))
                {
                    if (EvaluateMinterm(minterm, line, engine))
                    {
                        return false; // A line matched the negated pattern → fail
                    }
                }
                return true; // No line matched the negated pattern → pass
            }

            // Regular compound pattern logic
            std::istringstream stream(content);
            std::string line;

            while (std::getline(stream, line))
            {
                bool allMintermsPassed = true;

                for (const auto& [negated, minterm] : minterms)
                {
                    const bool match = EvaluateMinterm(minterm, line, engine);
                    if ((negated && match) || (!negated && !match))
                    {
                        allMintermsPassed = false;
                        break;
                    }
                }

                if (allMintermsPassed)
                {
                    return true; // A line satisfied all minterms
                }
            }

            return false; // No line satisfied all minterms
        }
        catch (const std::exception& e)
        {
            LogError("{}", e.what());
            return std::nullopt;
        }
    }

} // namespace sca
