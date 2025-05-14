#include <sca_policy_check.hpp>

#include <sca_utils.hpp>

#include <logger.hpp>
#include <registryHelper.hpp>

namespace
{
    std::pair<std::string, std::string> SplitRegistryKey(std::string_view fullKey)
    {
        if (fullKey.empty())
        {
            return {"", ""};
        }

        const size_t separator = fullKey.find('\\');

        if (separator == std::string_view::npos)
        {
            return {std::string(fullKey), ""};
        }
        return {std::string(fullKey.substr(0, separator)), std::string(fullKey.substr(separator + 1))};
    }

    // Checks if a registry key exists
    const RegistryRuleEvaluator::IsValidKeyFunc DEFAULT_IS_VALID_KEY = [](const std::string& rootKey) -> bool
    {
        try
        {
            const auto [key, subkey] = SplitRegistryKey(rootKey);
            return Utils::Registry::KeyExists(key, subkey);
        }
        catch (...)
        {
            return false;
        }
    };

    // Gets subkeys
    const RegistryRuleEvaluator::EnumKeysFunc DEFAULT_ENUM_KEYS =
        [](const std::string& root) -> std::vector<std::string>
    {
        try
        {
            const auto [key, path] = SplitRegistryKey(root);
            return Utils::Registry(key, path).enumerate();
        }
        catch (...)
        {
            return {};
        }
    };

    // Gets values from a key
    const RegistryRuleEvaluator::EnumValuesFunc DEFAULT_ENUM_VALUES =
        [](const std::string& root) -> std::vector<std::string>
    {
        try
        {
            const auto [key, path] = SplitRegistryKey(root);
            return Utils::Registry(key, path).enumerateValueKey();
        }
        catch (...)
        {
            return {};
        }
    };

    const RegistryRuleEvaluator::GetValueFunc DEFAULT_GET_VALUE =
        [](const std::string& root, const std::string& value) -> std::optional<std::string>
    {
        try
        {
            const auto [key, path] = SplitRegistryKey(root);
            return Utils::Registry(key, path).getValue(value);
        }
        catch (...)
        {
            return std::nullopt;
        }
    };

    RuleResult CheckMatch(const std::string& candidate, const std::string& pattern, bool isRegex)
    {
        if (isRegex)
        {
            const auto patternMatch = sca::PatternMatches(candidate, pattern);
            if (patternMatch.has_value() && patternMatch.value())
            {
                return RuleResult::Found;
            }
        }
        else if (candidate == pattern)
        {
            return RuleResult::Found;
        }

        return RuleResult::NotFound;
    }
} // namespace

RegistryRuleEvaluator::RegistryRuleEvaluator(PolicyEvaluationContext ctx,
                                             IsValidKeyFunc isValidKey,
                                             EnumKeysFunc enumKeys,
                                             EnumValuesFunc enumValues,
                                             GetValueFunc getValue)
    : RuleEvaluator(std::move(ctx), nullptr)
    , m_isValidKey(isValidKey ? isValidKey : DEFAULT_IS_VALID_KEY)
    , m_enumKeys(enumKeys ? enumKeys : DEFAULT_ENUM_KEYS)
    , m_enumValues(enumValues ? enumValues : DEFAULT_ENUM_VALUES)
    , m_getValue(getValue ? getValue : DEFAULT_GET_VALUE)
{
}

RuleResult RegistryRuleEvaluator::Evaluate()
{
    if (m_ctx.pattern)
    {
        return CheckKeyForContents();
    }
    return CheckKeyExistence();
}

RuleResult RegistryRuleEvaluator::CheckKeyForContents()
{
    LogDebug("Processing registry rule: {}", m_ctx.rule);

    // First check that the key exists
    if (!m_isValidKey(m_ctx.rule))
    {
        LogDebug("Key '{}' does not exist", m_ctx.rule);
        return RuleResult::Invalid;
    }

    auto result = RuleResult::NotFound;

    const auto pattern = *m_ctx.pattern; // NOLINT(bugprone-unchecked-optional-access)

    if (const auto content = sca::GetPattern(pattern))
    {
        const auto valueName = pattern.substr(0, m_ctx.pattern->find(" -> "));

        const auto obtainedValue = m_getValue(m_ctx.rule, valueName);

        // Check that the value exists
        if (!obtainedValue.has_value())
        {
            LogDebug("Value '{}' does not exist", valueName);
            return RuleResult::Invalid;
        }

        result = CheckMatch(obtainedValue.value(), content.value(), sca::IsRegexOrNumericPattern(content.value()));
    }
    else
    {
        // Will check for key or value existence
        const auto isRegex = sca::IsRegexPattern(pattern);

        for (const auto& key : m_enumKeys(m_ctx.rule))
        {
            if (CheckMatch(key, pattern, isRegex) == RuleResult::Found)
            {
                result = RuleResult::Found;
                break;
            }
        }

        if (result == RuleResult::NotFound)
        {
            for (const auto& value : m_enumValues(m_ctx.rule))
            {
                if (CheckMatch(value, pattern, isRegex) == RuleResult::Found)
                {
                    result = RuleResult::Found;
                    break;
                }
            }
        }
    }

    const RuleResult retVal =
        m_ctx.isNegated ? (result == RuleResult::Found ? RuleResult::NotFound : RuleResult::Found) : result;

    LogDebug("Registry rule evaluation {}", retVal == RuleResult::Found ? "passed" : "failed");
    return retVal;
}

RuleResult RegistryRuleEvaluator::CheckKeyExistence()
{
    auto result = RuleResult::NotFound;

    LogDebug("Checking existence of key: '{}'", m_ctx.rule);

    try
    {
        if (!m_isValidKey(m_ctx.rule))
        {
            LogDebug("Key '{}' does not exist", m_ctx.rule);
        }
        else
        {
            LogDebug("Key '{}' exists", m_ctx.rule);
            result = RuleResult::Found;
        }
    }
    catch (const std::exception& e)
    {
        LogDebug("RegistryRuleEvaluator::Evaluate: Exception: {}", e.what());
        return RuleResult::Invalid;
    }

    return m_ctx.isNegated ? (result == RuleResult::Found ? RuleResult::NotFound : RuleResult::Found) : result;
}
