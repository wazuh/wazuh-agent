#include <sca_policy_check.hpp>

#include <cmdHelper.hpp>
#include <filesystem_wrapper.hpp>

#include <vector>

CheckConditionEvaluator CheckConditionEvaluator::fromString(const std::string& str)
{
    if (str == "all")
    {
        return CheckConditionEvaluator {ConditionType::All};
    }
    if (str == "any")
    {
        return CheckConditionEvaluator {ConditionType::Any};
    }
    if (str == "none")
    {
        return CheckConditionEvaluator {ConditionType::None};
    }
    throw std::invalid_argument("Invalid condition type: " + str);
}

CheckConditionEvaluator::CheckConditionEvaluator(ConditionType type)
    : m_type {type}
{
}

void CheckConditionEvaluator::addResult(bool passed)
{
    if (m_result.has_value())
    {
        return;
    }

    ++m_totalRules;
    m_passedRules += passed;

    switch (m_type)
    {
        case ConditionType::All:
            if (!passed)
            {
                m_result = false;
            }
            break;
        case ConditionType::Any:
            if (passed)
            {
                m_result = true;
            }
            break;
        case ConditionType::None:
            if (passed)
            {
                m_result = false;
            }
            break;
    }
}

bool CheckConditionEvaluator::result() const
{
    if (m_result.has_value())
    {
        return *m_result;
    }

    switch (m_type)
    {
        case ConditionType::All: return m_totalRules > 0 && m_passedRules == m_totalRules;
        case ConditionType::Any: return m_passedRules > 0;
        case ConditionType::None: return m_passedRules == 0;
    }

    return false;
}

RuleEvaluator::RuleEvaluator(const PolicyEvaluationContext& ctx, std::unique_ptr<IFileSystemWrapper> fileSystemWrapper)
    : m_fileSystemWrapper(fileSystemWrapper ? std::move(fileSystemWrapper)
                                            : std::make_unique<file_system::FileSystemWrapper>())
    , m_ctx(ctx)
{
    if (m_ctx.rule.empty())
    {
        throw std::invalid_argument("Rule cannot be empty");
    }
}

const PolicyEvaluationContext& RuleEvaluator::GetContext() const
{
    return m_ctx;
}

FileRuleEvaluator::FileRuleEvaluator(const PolicyEvaluationContext& ctx,
                                     std::unique_ptr<IFileSystemWrapper> fileSystemWrapper)
    : RuleEvaluator(ctx, std::move(fileSystemWrapper))
{
}

RuleResult FileRuleEvaluator::Evaluate()
{
    if (m_ctx.pattern)
    {
        return CheckFileForContents();
    }
    return CheckFileExistence();
}

RuleResult FileRuleEvaluator::CheckFileForContents()
{
    if (m_fileSystemWrapper->exists(m_ctx.rule))
    {
        // Check file contents against the pattern
        // Placeholder for actual content check logic
        return RuleResult::Found;
    }
    return RuleResult::NotFound; // or invalid?
}

RuleResult FileRuleEvaluator::CheckFileExistence()
{
    if (m_fileSystemWrapper->exists(m_ctx.rule))
    {
        // Check file contents against the pattern
        // Placeholder for actual content check logic
        return RuleResult::Found;
    }
    return RuleResult::NotFound;
}

CommandRuleEvaluator::CommandRuleEvaluator(const PolicyEvaluationContext& ctx,
                                           std::unique_ptr<IFileSystemWrapper> fileSystemWrapper)
    : RuleEvaluator(ctx, std::move(fileSystemWrapper))
{
}

RuleResult CommandRuleEvaluator::Evaluate()
{
    const auto output = Utils::Exec(m_ctx.rule);
    if (!output.empty())
    {
        // check pattern against output
        // Placeholder for actual pattern check logic
        return RuleResult::Found;
    }
    return RuleResult::NotFound;
}

DirRuleEvaluator::DirRuleEvaluator(const PolicyEvaluationContext& ctx,
                                   std::unique_ptr<IFileSystemWrapper> fileSystemWrapper)
    : RuleEvaluator(ctx, std::move(fileSystemWrapper))
{
}

RuleResult DirRuleEvaluator::Evaluate()
{
    if (!m_fileSystemWrapper->exists(m_ctx.rule) || !m_fileSystemWrapper->is_directory(m_ctx.rule))
    {
        return RuleResult::NotFound;
    }
    // check if pattern matches
    // Placeholder for actual pattern check logic
    return RuleResult::Found;
}

ProcessRuleEvaluator::ProcessRuleEvaluator(const PolicyEvaluationContext& ctx,
                                           std::unique_ptr<IFileSystemWrapper> fileSystemWrapper)
    : RuleEvaluator(ctx, std::move(fileSystemWrapper))
{
}

RuleResult ProcessRuleEvaluator::Evaluate()
{
    // get list of running processes
    // check if pattern matches
    return RuleResult::Invalid;
}
