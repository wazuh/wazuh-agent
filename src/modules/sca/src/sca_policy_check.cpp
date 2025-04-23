#include <sca_policy_check.hpp>

#include <cmdHelper.hpp>
#include <filesystem_wrapper.hpp>

#include <vector>

RuleEvaluator::RuleEvaluator(PolicyEvaluationContext ctx, std::unique_ptr<IFileSystemWrapper> fileSystemWrapper)
    : m_fileSystemWrapper(fileSystemWrapper ? std::move(fileSystemWrapper)
                                            : std::make_unique<file_system::FileSystemWrapper>())
    , m_ctx(std::move(ctx))
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

FileRuleEvaluator::FileRuleEvaluator(PolicyEvaluationContext ctx,
                                     std::unique_ptr<IFileSystemWrapper> fileSystemWrapper,
                                     std::unique_ptr<IFileIOUtils> fileUtils)
    : RuleEvaluator(std::move(ctx), std::move(fileSystemWrapper))
    , m_fileUtils(std::move(fileUtils))
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
        const auto pattern = *m_ctx.pattern; // NOLINT(bugprone-unchecked-optional-access)

        if (pattern.starts_with("r:") || pattern.starts_with("n:"))
        {
            const auto content = m_fileUtils->getFileContent(m_ctx.rule);

            if (sca::PatternMatches(content, pattern))
            {
                return RuleResult::Found;
            }
            return RuleResult::NotFound;
        }
        else
        {
            auto result = RuleResult::NotFound;

            m_fileUtils->readLineByLine(m_ctx.rule,
                                        [&pattern, &result](const std::string& line)
                                        {
                                            if (line == pattern)
                                            {
                                                // stop reading
                                                result = RuleResult::Found;
                                                return false;
                                            }
                                            // continue reading
                                            return true;
                                        });
            return result;
        }

        return RuleResult::NotFound;
    }
    return RuleResult::NotFound; // or invalid?
}

RuleResult FileRuleEvaluator::CheckFileExistence()
{
    if (m_fileSystemWrapper->exists(m_ctx.rule))
    {
        return RuleResult::Found;
    }
    return RuleResult::NotFound;
}

CommandRuleEvaluator::CommandRuleEvaluator(PolicyEvaluationContext ctx,
                                           std::unique_ptr<IFileSystemWrapper> fileSystemWrapper)
    : RuleEvaluator(std::move(ctx), std::move(fileSystemWrapper))
{
}

RuleResult CommandRuleEvaluator::Evaluate()
{
    if (const auto output = Utils::Exec(m_ctx.rule); !output.empty())
    {
        if (m_ctx.pattern)
        {
            if (m_ctx.pattern->starts_with("r:") || m_ctx.pattern->starts_with("n:"))
            {
                if (sca::PatternMatches(output, *m_ctx.pattern))
                {
                    return RuleResult::Found;
                }
            }
            else if (output == *m_ctx.pattern)
            {
                return RuleResult::Found;
            }
        }
    }
    return RuleResult::NotFound;
}

DirRuleEvaluator::DirRuleEvaluator(PolicyEvaluationContext ctx,
                                   std::unique_ptr<IFileSystemWrapper> fileSystemWrapper,
                                   std::unique_ptr<IFileIOUtils> fileUtils)
    : RuleEvaluator(std::move(ctx), std::move(fileSystemWrapper))
    , m_fileUtils(std::move(fileUtils))
{
}

RuleResult DirRuleEvaluator::Evaluate()
{
    if (!m_fileSystemWrapper->exists(m_ctx.rule) || !m_fileSystemWrapper->is_directory(m_ctx.rule))
    {
        return RuleResult::NotFound;
    }

    if (m_ctx.pattern)
    {
        if (m_ctx.pattern->starts_with("r:"))
        {
            // Check if a directory contains files that match a regex
            const auto files = m_fileSystemWrapper->list_directory(m_ctx.rule);

            for (const auto& file : files)
            {
                if (sca::PatternMatches(file.string(), *m_ctx.pattern))
                {
                    return RuleResult::Found;
                }
            }

            return RuleResult::NotFound;
        }
        else if (const auto content = sca::GetPattern(*m_ctx.pattern))
        {
            // Check files matching fileName for content
            const auto fileName = m_ctx.pattern->substr(0, m_ctx.pattern->find(" -> "));
            const auto files = m_fileSystemWrapper->list_directory(m_ctx.rule);

            for (const auto& file : files)
            {
                if (file.string() == fileName)
                {
                    // Check file content against the pattern
                    auto result = RuleResult::NotFound;

                    m_fileUtils->readLineByLine(m_ctx.rule,
                                                [&content, &result](const std::string& line)
                                                {
                                                    if (line == content)
                                                    {
                                                        // stop reading
                                                        result = RuleResult::Found;
                                                        return false;
                                                    }
                                                    // continue reading
                                                    return true;
                                                });
                    return result;
                }
            }
            return RuleResult::NotFound;
        }
        else
        {
            // Check if a directory contains files that match a string
            const auto pattern = *m_ctx.pattern; // NOLINT(bugprone-unchecked-optional-access)
            const auto files = m_fileSystemWrapper->list_directory(m_ctx.rule);
            for (const auto& file : files)
            {
                if (file.string() == pattern)
                {
                    return RuleResult::Found;
                }
            }
            return RuleResult::NotFound;
        }
    }
    return RuleResult::Found;
}

ProcessRuleEvaluator::ProcessRuleEvaluator(PolicyEvaluationContext ctx,
                                           std::unique_ptr<IFileSystemWrapper> fileSystemWrapper)
    : RuleEvaluator(std::move(ctx), std::move(fileSystemWrapper))
{
}

RuleResult ProcessRuleEvaluator::Evaluate()
{
    // get list of running processes
    // check if pattern matches
    return RuleResult::Invalid;
}
