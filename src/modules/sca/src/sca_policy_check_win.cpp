#include <sca_policy_check.hpp>

#include <registryHelper.hpp>
#include <sca_utils.hpp>

#include <logger.hpp>

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

    // Gets subkeys
    const RegistryRuleEvaluator::GetRegistryKeysFunc DEFAULT_GET_REGISTRY_KEYS =
        [](const std::string& root, const std::string& subkey) -> std::vector<std::string>
    {
        return Utils::Registry(root, subkey).enumerate();
    };

    // Gets values from a key
    const RegistryRuleEvaluator::GetRegistryValuesFunc DEFAULT_GET_REGISTRY_VALUES =
        [](const std::string& root, const std::string& subkey) -> std::vector<std::string>
    {
        return Utils::Registry(root, subkey).enumerateValueKey();
    };

} // namespace

RegistryRuleEvaluator::RegistryRuleEvaluator(PolicyEvaluationContext ctx,
                                             IsValidRegistryKeyFunc isValidRegistryKey,
                                             GetRegistryKeysFunc getRegistryKeys,
                                             GetRegistryValuesFunc getRegistryValues)
    : RuleEvaluator(std::move(ctx), nullptr)
    , m_isValidRegistryKey(isValidRegistryKey ? std::move(isValidRegistryKey) : DEFAULT_IS_VALID_REGISTRY_KEY)
    , m_getRegistryKeys(getRegistryKeys ? std::move(getRegistryKeys) : DEFAULT_GET_REGISTRY_KEYS)
    , m_getRegistryValues(getRegistryValues ? std::move(getRegistryValues) : DEFAULT_GET_REGISTRY_VALUES)
{
}

RuleResult RegistryRuleEvaluator::Evaluate()
{
    LogDebug("Processing registry rule: {}", m_ctx.rule);

    auto result = RuleResult::NotFound;

    try
    {
        if (m_ctx.pattern)
        {
            if (const auto content = sca::GetPattern(*m_ctx.pattern))
            {
                const auto registryKey = m_ctx.pattern->substr(0, m_ctx.pattern->find(" -> "));

                for (const auto& value : m_getRegistryValues(m_ctx.rule, registryKey))
                {
                    if (value == *content)
                    {
                        result = RuleResult::Found;
                        break;
                    }
                }
            }
            else
            {
                for (const auto& key : m_getRegistryKeys(m_ctx.rule, *m_ctx.pattern))
                {
                    if (key == *m_ctx.pattern)
                    {
                        result = RuleResult::Found;
                        break;
                    }
                }
            }
        }
        else if (m_isValidRegistryKey(m_ctx.rule))
        {
            result = RuleResult::Found;
        }
    }
    catch (const std::exception& e)
    {
        LogDebug("RegistryRuleEvaluator::Evaluate: Exception: {}", e.what());
        return RuleResult::Invalid;
    }

    LogDebug("Registry rule evaluation {}", result == RuleResult::Found ? "passed" : "failed");
    return m_ctx.isNegated ? (result == RuleResult::Found ? RuleResult::NotFound : RuleResult::Found) : result;
}
