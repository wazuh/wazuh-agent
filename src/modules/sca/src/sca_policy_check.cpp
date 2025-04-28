#include <sca_policy_check.hpp>

#include <cmdHelper.hpp>
#include <file_io_utils.hpp>
#include <filesystem_wrapper.hpp>
#include <os_utils.hpp>
#include <sca_utils.hpp>
#include <stringHelper.hpp>

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
    if (!m_fileSystemWrapper->exists(m_ctx.rule) || !m_fileSystemWrapper->is_regular_file(m_ctx.rule))
    {
        return RuleResult::Invalid;
    }

    const auto pattern = *m_ctx.pattern; // NOLINT(bugprone-unchecked-optional-access)

    bool matchFound = false;

    if (pattern.starts_with("r:") || pattern.starts_with("n:"))
    {
        const auto content = m_fileUtils->getFileContent(m_ctx.rule);
        matchFound = sca::PatternMatches(content, pattern);
    }
    else
    {
        m_fileUtils->readLineByLine(m_ctx.rule,
                                    [&pattern, &matchFound](const std::string& line)
                                    {
                                        if (line == pattern)
                                        {
                                            // stop reading
                                            matchFound = true;
                                            return false;
                                        }
                                        // continue reading
                                        return true;
                                    });
    }

    return (matchFound != m_ctx.isNegated) ? RuleResult::Found : RuleResult::NotFound;
}

RuleResult FileRuleEvaluator::CheckFileExistence()
{
    const auto exists = m_fileSystemWrapper->exists(m_ctx.rule) && m_fileSystemWrapper->is_regular_file(m_ctx.rule);
    const auto result = exists ? RuleResult::Found : RuleResult::NotFound;

    return m_ctx.isNegated ? (result == RuleResult::Found ? RuleResult::NotFound : RuleResult::Found) : result;
}

CommandRuleEvaluator::CommandRuleEvaluator(PolicyEvaluationContext ctx,
                                           std::unique_ptr<IFileSystemWrapper> fileSystemWrapper,
                                           CommandExecFunc commandExecFunc)
    : RuleEvaluator(std::move(ctx), std::move(fileSystemWrapper))
    , m_commandExecFunc(commandExecFunc
                        ? std::move(commandExecFunc)
                        : [](const std::string& cmd) { const auto cmdOutput = Utils::Exec(cmd); return cmdOutput.StdOut + cmdOutput.StdErr; })
{
}

RuleResult CommandRuleEvaluator::Evaluate()
{
    RuleResult result = RuleResult::NotFound;

    if (const auto output = m_commandExecFunc(m_ctx.rule); !output.empty())
    {
        if (m_ctx.pattern)
        {
            if (m_ctx.pattern->starts_with("r:") || m_ctx.pattern->starts_with("n:"))
            {
                if (sca::PatternMatches(output, *m_ctx.pattern))
                {
                    result = RuleResult::Found;
                }
            }
            else if (output == *m_ctx.pattern)
            {
                result = RuleResult::Found;
            }
        }
    }

    return m_ctx.isNegated ? (result == RuleResult::Found ? RuleResult::NotFound : RuleResult::Found) : result;
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
    auto result = RuleResult::NotFound;

    if (!m_fileSystemWrapper->exists(m_ctx.rule) || !m_fileSystemWrapper->is_directory(m_ctx.rule))
    {
        result = RuleResult::NotFound;
    }
    else if (m_ctx.pattern)
    {
        if (m_ctx.pattern->starts_with("r:"))
        {
            const auto files = m_fileSystemWrapper->list_directory(m_ctx.rule);

            for (const auto& file : files)
            {
                if (sca::PatternMatches(file.string(), *m_ctx.pattern))
                {
                    result = RuleResult::Found;
                    break;
                }
            }
        }
        else if (const auto content = sca::GetPattern(*m_ctx.pattern))
        {
            const auto fileName = m_ctx.pattern->substr(0, m_ctx.pattern->find(" -> "));
            const auto files = m_fileSystemWrapper->list_directory(m_ctx.rule);

            for (const auto& file : files)
            {
                if (file.string() == fileName)
                {
                    result = RuleResult::NotFound;

                    m_fileUtils->readLineByLine(m_ctx.rule,
                                                [&content, &result](const std::string& line)
                                                {
                                                    if (line == content)
                                                    {
                                                        result = RuleResult::Found;
                                                        return false;
                                                    }
                                                    return true;
                                                });
                    break;
                }
            }
        }
        else
        {
            const auto pattern = *m_ctx.pattern;
            const auto files = m_fileSystemWrapper->list_directory(m_ctx.rule);

            for (const auto& file : files)
            {
                if (file.string() == pattern)
                {
                    result = RuleResult::Found;
                    break;
                }
            }
        }
    }
    else
    {
        result = RuleResult::Found;
    }

    return m_ctx.isNegated ? (result == RuleResult::Found ? RuleResult::NotFound : RuleResult::Found) : result;
}

ProcessRuleEvaluator::ProcessRuleEvaluator(PolicyEvaluationContext ctx,
                                           std::unique_ptr<IFileSystemWrapper> fileSystemWrapper,
                                           GetProcessesFunc getProcesses)
    : RuleEvaluator(std::move(ctx), std::move(fileSystemWrapper))
    , m_getProcesses(getProcesses ? std::move(getProcesses) : [] { return os_utils::OsUtils().GetRunningProcesses(); })
{
}

RuleResult ProcessRuleEvaluator::Evaluate()
{
    const auto processes = m_getProcesses();
    RuleResult result = RuleResult::NotFound;

    for (const auto& process : processes)
    {
        if (process == m_ctx.rule)
        {
            result = RuleResult::Found;
            break;
        }
    }

    return m_ctx.isNegated ? (result == RuleResult::Found ? RuleResult::NotFound : RuleResult::Found) : result;
}

std::unique_ptr<IRuleEvaluator>
RuleEvaluatorFactory::CreateEvaluator(const std::string& input,
                                      std::unique_ptr<IFileSystemWrapper> fileSystemWrapper,
                                      std::unique_ptr<IFileIOUtils> fileUtils)
{
    if (!fileSystemWrapper)
    {
        fileSystemWrapper = std::make_unique<file_system::FileSystemWrapper>();
    }
    if (!fileUtils)
    {
        fileUtils = std::make_unique<file_io::FileIOUtils>();
    }

    auto ruleInput = Utils::Trim(input, " \t");
    auto isNegated = false;
    if (ruleInput.starts_with("not "))
    {
        isNegated = true;
        ruleInput = Utils::Trim(ruleInput.substr(4), " \t");
    }

    const auto pattern = sca::GetPattern(ruleInput);
    if (pattern.has_value())
    {
        ruleInput = Utils::Trim(ruleInput.substr(0, ruleInput.find("->")), " \t");
    }

    const auto ruleTypeAndValue = sca::ParseRuleType(ruleInput);
    if (!ruleTypeAndValue.has_value())
    {
        return nullptr;
    }

    const auto [ruleType, cleanedRule] = ruleTypeAndValue.value();

    const PolicyEvaluationContext ctx {.rule = cleanedRule, .pattern = pattern, .isNegated = isNegated};

    switch (ruleType)
    {
        case sca::WM_SCA_TYPE_FILE:
            return std::make_unique<FileRuleEvaluator>(ctx, std::move(fileSystemWrapper), std::move(fileUtils));
#ifdef _WIN32
        case sca::WM_SCA_TYPE_REGISTRY: return std::make_unique<RegistryRuleEvaluator>(ctx);
#endif
        case sca::WM_SCA_TYPE_PROCESS: return std::make_unique<ProcessRuleEvaluator>(ctx, std::move(fileSystemWrapper));
        case sca::WM_SCA_TYPE_DIR:
            return std::make_unique<DirRuleEvaluator>(ctx, std::move(fileSystemWrapper), std::move(fileUtils));
        case sca::WM_SCA_TYPE_COMMAND: return std::make_unique<CommandRuleEvaluator>(ctx, std::move(fileSystemWrapper));
        default: return nullptr;
    }
}
