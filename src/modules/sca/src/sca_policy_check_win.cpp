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
    const RegistryRuleEvaluator::IsValidKeyFunc DEFAULT_IS_VALID_KEY = [](const std::string& rootKey) -> bool
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

    // Gets subkeys
    const RegistryRuleEvaluator::EnumKeysFunc DEFAULT_ENUM_KEYS =
        [](const std::string& root) -> std::vector<std::string>
    {
        return Utils::Registry(rootKey).enumerateValueKey();
    };

    // Gets values from a key
    const RegistryRuleEvaluator::EnumValuesFunc DEFAULT_ENUM_VALUES =
        [](const std::string& root) -> std::vector<std::string>
    {
        auto [key, path] = SplitRegistryKey(root);

        return Utils::Registry(key, path).enumerateValueKey();
    };
} // namespace

RegistryRuleEvaluator::RegistryRuleEvaluator(PolicyEvaluationContext ctx,
                                             IsValidKeyFunc isValidKey,
                                             EnumKeysFunc enumKeys,
                                             EnumValuesFunc enumValues)
    : RuleEvaluator(std::move(ctx), nullptr)
    , m_isValidKey(isValidKey ? std::move(isValidKey) : DEFAULT_IS_VALID_KEY)
    , m_enumKeys(enumKeys ? std::move(enumKeys) : DEFAULT_ENUM_KEYS)
    , m_enumValues(enumValues ? std::move(enumValues) : DEFAULT_ENUM_VALUES)
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

    const auto pattern = *m_ctx.pattern; // NOLINT(bugprone-unchecked-optional-access)

    try
    {
        if (!m_isValidRegistryKey(m_ctx.rule))
        {
            LogDebug("Key '{}' does not exist", m_ctx.rule);
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
                        LogDebug("Pattern '{}' was found in key '{}'", pattern, m_ctx.rule);
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
                    LogDebug("Pattern '{}' was found in key '{}'", pattern, m_ctx.rule);
                    return m_ctx.isNegated ? RuleResult::NotFound : RuleResult::Found;
                }
            }
        }

        if (isRegex && !hadValue)
        {
            LogDebug("Invalid pattern '{}' for key '{}'", pattern, m_ctx.rule);
            return RuleResult::Invalid;
        }
    }
    catch (const std::exception& e)
    {
        LogDebug("RegistryRuleEvaluator::Evaluate: Exception: {}", e.what());
        return RuleResult::Invalid;
    }

    LogDebug("Pattern '{}' was not found in key '{}'", pattern, m_ctx.rule);
    return m_ctx.isNegated ? RuleResult::Found : RuleResult::NotFound;
}

RuleResult RegistryRuleEvaluator::CheckKeyExistence()
{
    auto result = RuleResult::NotFound;

    LogDebug("Processing registry rule. Checking existence of key: '{}'", m_ctx.rule);

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
