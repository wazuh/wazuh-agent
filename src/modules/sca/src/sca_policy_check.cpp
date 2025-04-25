#include <sca_policy_check.hpp>

#include <cmdHelper.hpp>
#include <filesystem_wrapper.hpp>
#include <os_utils.hpp>
#include <sca_utils.hpp>

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
    if (!m_fileSystemWrapper->exists(m_ctx.rule))
    {
        return m_ctx.isNegated ? RuleResult::Found : RuleResult::NotFound;
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
    const bool exists = m_fileSystemWrapper->exists(m_ctx.rule);
    const RuleResult result = exists ? RuleResult::Found : RuleResult::NotFound;

    return m_ctx.isNegated ? (result == RuleResult::Found ? RuleResult::NotFound : RuleResult::Found) : result;
}

CommandRuleEvaluator::CommandRuleEvaluator(PolicyEvaluationContext ctx,
                                           std::unique_ptr<IFileSystemWrapper> fileSystemWrapper,
                                           CommandExecFunc commandExecFunc)
    : RuleEvaluator(std::move(ctx), std::move(fileSystemWrapper))
    , m_commandExecFunc(commandExecFunc ? std::move(commandExecFunc) : [](const std::string& cmd)
                            { return Utils::Exec(cmd); })
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
    auto trim = [](const std::string& str) -> std::string
    {
        const auto start = str.find_first_not_of(" \t");
        const auto end = str.find_last_not_of(" \t");
        return (start == std::string::npos) ? "" : str.substr(start, end - start + 1);
    };

    const auto arrowPos = input.find("->");
    if (arrowPos == std::string::npos)
    {
        return nullptr;
    }

    std::string rule = trim(input.substr(0, arrowPos));
    const std::string pattern = trim(input.substr(arrowPos + 2));

    bool isNegated = false;
    if (rule.starts_with("not "))
    {
        isNegated = true;
        rule = trim(rule.substr(4));
    }

    const auto delimiter_pos = rule.find(':');
    if (delimiter_pos == std::string::npos)
    {
        return nullptr;
    }

    auto key = rule.substr(0, delimiter_pos);

    if (!key.empty() && key.front() == '!')
    {
        key.erase(0, 1);
    }

    static const std::map<std::string, int> type_map = {{"f", sca::WM_SCA_TYPE_FILE},
                                                        {"r", sca::WM_SCA_TYPE_REGISTRY},
                                                        {"p", sca::WM_SCA_TYPE_PROCESS},
                                                        {"d", sca::WM_SCA_TYPE_DIR},
                                                        {"c", sca::WM_SCA_TYPE_COMMAND}};

    const auto it = type_map.find(key);
    if (it == type_map.end())
    {
        return nullptr;
    }

    const std::string cleanedRule = rule.substr(delimiter_pos + 1);

    PolicyEvaluationContext ctx;
    ctx.rule = cleanedRule;
    ctx.pattern = pattern.empty() ? std::nullopt : std::make_optional(pattern);
    ctx.isNegated = isNegated;

    switch (it->second)
    {
        case sca::WM_SCA_TYPE_FILE:
            return std::make_unique<FileRuleEvaluator>(ctx, std::move(fileSystemWrapper), std::move(fileUtils));
        case sca::WM_SCA_TYPE_REGISTRY:
            return std::make_unique<RegistryRuleEvaluator>(ctx, std::move(fileSystemWrapper));
        case sca::WM_SCA_TYPE_PROCESS: return std::make_unique<ProcessRuleEvaluator>(ctx, std::move(fileSystemWrapper));
        case sca::WM_SCA_TYPE_DIR:
            return std::make_unique<DirRuleEvaluator>(ctx, std::move(fileSystemWrapper), std::move(fileUtils));
        case sca::WM_SCA_TYPE_COMMAND: return std::make_unique<CommandRuleEvaluator>(ctx, std::move(fileSystemWrapper));
        default: return nullptr;
    }
}
