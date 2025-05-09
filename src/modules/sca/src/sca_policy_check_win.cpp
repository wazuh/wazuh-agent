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

        size_t separator = fullKey.find('\\');

        if (separator == std::string_view::npos)
        {
            return {std::string(fullKey), ""};
        }
        return {std::string(fullKey.substr(0, separator)), std::string(fullKey.substr(separator + 1))};
    }

    // Checks if a registry key exists
    const RegistryRuleEvaluator::IsValidRegistryKeyFunc DEFAULT_IS_VALID_REGISTRY_KEY =
        [](const std::string& rootKey) -> bool
    {
        try
        {
            auto [key, subkey] = SplitRegistryKey(rootKey);
            return Utils::Registry::KeyExists(key, subkey);
        }
        catch (...)
        {
            return false;
        }
    };

    // Gets values from a key
    const RegistryRuleEvaluator::GetRegistryValuesFunc DEFAULT_GET_REGISTRY_VALUES =
        [](const std::string& rootKey) -> std::vector<std::string>
    {
        return Utils::Registry(rootKey).enumerateValueKey();
    };

    // Gets value from a key
    const RegistryRuleEvaluator::GetRegistryKeyValueFunc DEFAULT_GET_REGISTRY_KEY_VALUE =
        [](const std::string& rootKey, const std::string& key) -> std::string
    {
        return Utils::Registry(rootKey).getValue(key);
    };

    RuleResult FindContentInRegistryValue(const std::string& value, const std::string& pattern, const bool isNegated)
    {
        bool matchFound = false;

        if (sca::IsRegexOrNumericPattern(pattern))
        {
            if (const auto patternMatch = sca::PatternMatches(value, pattern))
            {
                matchFound = patternMatch.value();
            }
            else
            {
                LogDebug("Invalid pattern '{}' for registry value '{}'", pattern, value);
                return RuleResult::Invalid;
            }
        }
        else
        {
            if (value == pattern)
            {
                matchFound = true;
            }
        }

        LogDebug("Pattern '{}' {} found in registry value '{}'", pattern, matchFound ? "was" : "was not", value);
        return (matchFound != isNegated) ? RuleResult::Found : RuleResult::NotFound;
    }
} // namespace

RegistryRuleEvaluator::RegistryRuleEvaluator(PolicyEvaluationContext ctx,
                                             IsValidRegistryKeyFunc isValidRegistryKey,
                                             GetRegistryValuesFunc getRegistryValues,
                                             GetRegistryKeyValueFunc getRegistryKeyValue)
    : RuleEvaluator(std::move(ctx), nullptr)
    , m_isValidRegistryKey(isValidRegistryKey ? std::move(isValidRegistryKey) : DEFAULT_IS_VALID_REGISTRY_KEY)
    , m_getRegistryValues(getRegistryValues ? std::move(getRegistryValues) : DEFAULT_GET_REGISTRY_VALUES)
    , m_getRegistryKeyValue(getRegistryKeyValue ? std::move(getRegistryKeyValue) : DEFAULT_GET_REGISTRY_KEY_VALUE)
{
}

RuleResult RegistryRuleEvaluator::Evaluate()
{
    if (m_ctx.pattern)
    {
        return CheckRegistryForContents();
    }
    return CheckRegistryExistence();
}

RuleResult RegistryRuleEvaluator::CheckRegistryForContents()
{
    LogDebug("Processing registry rule: {}", m_ctx.rule);

    const auto pattern = *m_ctx.pattern; // NOLINT(bugprone-unchecked-optional-access)

    try
    {
        if (!m_isValidRegistryKey(m_ctx.rule))
        {
            LogDebug("Registry '{}' does not exist", m_ctx.rule);
            return RuleResult::Invalid;
        }

        bool hadValue = false;

        // Check if pattern is a regex
        const auto isRegex = sca::IsRegexPattern(pattern);

        // Check if pattern has content
        const auto content = sca::GetPattern(pattern);

        for (const auto& value : m_getRegistryValues(m_ctx.rule))
        {
            if (isRegex)
            {
                const auto patternMatch = sca::PatternMatches(value, pattern);
                if (patternMatch.has_value())
                {
                    hadValue = true;
                    if (patternMatch.value())
                    {
                        LogDebug("Pattern '{}' was found in registry '{}'", pattern, m_ctx.rule);
                        return m_ctx.isNegated ? RuleResult::NotFound : RuleResult::Found;
                    }
                }
            }
            else if (content.has_value())
            {
                const auto registryKey = m_ctx.pattern->substr(0, m_ctx.pattern->find(" -> "));

                if (value == registryKey)
                {
                    return FindContentInRegistryValue(
                        m_getRegistryKeyValue(m_ctx.rule, registryKey), content.value(), m_ctx.isNegated);
                }
            }
            else
            {
                if (value == pattern)
                {
                    LogDebug("Pattern '{}' was found in registry '{}'", pattern, m_ctx.rule);
                    return m_ctx.isNegated ? RuleResult::NotFound : RuleResult::Found;
                }
            }
        }

        if (isRegex && !hadValue)
        {
            LogDebug("Invalid pattern '{}' for registry '{}'", pattern, m_ctx.rule);
            return RuleResult::Invalid;
        }
    }
    catch (const std::exception& e)
    {
        LogDebug("RegistryRuleEvaluator::Evaluate: Exception: {}", e.what());
        return RuleResult::Invalid;
    }

    LogDebug("Pattern '{}' was not found in registry '{}'", pattern, m_ctx.rule);
    return m_ctx.isNegated ? RuleResult::Found : RuleResult::NotFound;
}

RuleResult RegistryRuleEvaluator::CheckRegistryExistence()
{
    auto result = RuleResult::NotFound;

    LogDebug("Processing registry rule. Checking existence of registry: '{}'", m_ctx.rule);

    try
    {
        if (!m_isValidRegistryKey(m_ctx.rule))
        {
            LogDebug("Registry '{}' does not exist", m_ctx.rule);
        }
        else
        {
            LogDebug("Registry '{}' exists", m_ctx.rule);
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
