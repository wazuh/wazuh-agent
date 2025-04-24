#include <sca_policy_check.hpp>

RegistryRuleEvaluator::RegistryRuleEvaluator(PolicyEvaluationContext ctx,
                                             std::unique_ptr<IFileSystemWrapper> fileSystemWrapper)
    : RuleEvaluator(std::move(ctx), std::move(fileSystemWrapper))
{
}

RuleResult RegistryRuleEvaluator::Evaluate()
{
    return RuleResult::Invalid;
}
