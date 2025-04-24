#include <sca_policy_check.hpp>

#include <registryHelper.hpp>
#include <sca_utils.hpp>

#include <logger.hpp>

RegistryRuleEvaluator::RegistryRuleEvaluator(PolicyEvaluationContext ctx,
                                             std::unique_ptr<IFileSystemWrapper> fileSystemWrapper)
    : RuleEvaluator(std::move(ctx), std::move(fileSystemWrapper))
{
}

RuleResult RegistryRuleEvaluator::Evaluate()
{
    try
    {
        if (m_ctx.pattern)
        {
            if (const auto content = sca::GetPattern(*m_ctx.pattern))
            {
                const auto registryKey = m_ctx.pattern->substr(0, m_ctx.pattern->find(" -> "));

                Utils::Registry registry(m_ctx.rule, registryKey);
                for (const auto& value : registry.enumerateValueKey())
                {
                    if (value == *content)
                    {
                        return RuleResult::Found;
                    }
                }
                return RuleResult::NotFound;
            }
            else
            {
                Utils::Registry registry(m_ctx.rule, *m_ctx.pattern);
                for (const auto& key : registry.enumerate())
                {
                    if (key == *m_ctx.pattern)
                    {
                        return RuleResult::Found;
                    }
                }

                return RuleResult::NotFound;
            }
        }

        // If there's no pattern we are just checking that the registry exists
        // Since Utils::Registry will throw if it doesn't exist, we can just return Found
        [[maybe_unused]] Utils::Registry registry(m_ctx.rule);
        return RuleResult::Found;
    }
    catch (const std::exception& e)
    {
        LogDebug("RegistryRuleEvaluator::Evaluate: Exception: {}", e.what());
    }
    return RuleResult::NotFound;
}
