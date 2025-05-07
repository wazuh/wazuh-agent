#include <sca_policy_check.hpp>

#include <cmdHelper.hpp>
#include <file_io_utils.hpp>
#include <filesystem_wrapper.hpp>
#include <logger.hpp>
#include <sca_utils.hpp>
#include <stringHelper.hpp>
#include <sysInfo.hpp>
#include <sysInfoInterface.hpp>

namespace
{
    bool IsRegexPattern(const std::string& pattern)
    {
        return pattern.starts_with("r:") || pattern.starts_with("!r:");
    }

    bool IsRegexOrNumericPattern(const std::string& pattern)
    {
        return IsRegexPattern(pattern) || pattern.starts_with("n:") || pattern.starts_with("!n:");
    }

} // namespace

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
    // NOLINTNEXTLINE(bugprone-unchecked-optional-access)
    const auto pattern = *m_ctx.pattern;

    LogDebug("Processing file rule. Checking contents of file: '{}' against pattern '{}'", m_ctx.rule, pattern);

    if (!m_fileSystemWrapper->exists(m_ctx.rule) || !m_fileSystemWrapper->is_regular_file(m_ctx.rule))
    {
        LogDebug("File '{}' does not exist or is not a regular file", m_ctx.rule);
        return RuleResult::Invalid;
    }

    bool matchFound = false;

    if (IsRegexOrNumericPattern(pattern))
    {
        const auto content = m_fileUtils->getFileContent(m_ctx.rule);

        if (const auto patternMatch = sca::PatternMatches(content, pattern))
        {
            matchFound = patternMatch.value();
        }
        else
        {
            return RuleResult::Invalid;
        }
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

    LogDebug("Pattern '{}' {} found in file '{}'", pattern, matchFound ? "was" : "was not", m_ctx.rule);
    return (matchFound != m_ctx.isNegated) ? RuleResult::Found : RuleResult::NotFound;
}

RuleResult FileRuleEvaluator::CheckFileExistence()
{
    LogDebug("Processing file rule. Checking existence of file: '{}'", m_ctx.rule);
    const auto exists = m_fileSystemWrapper->exists(m_ctx.rule) && m_fileSystemWrapper->is_regular_file(m_ctx.rule);
    const auto result = exists ? RuleResult::Found : RuleResult::NotFound;

    LogDebug("File '{}' {} found", m_ctx.rule, exists ? "was" : "was not");
    return m_ctx.isNegated ? (result == RuleResult::Found ? RuleResult::NotFound : RuleResult::Found) : result;
}

CommandRuleEvaluator::CommandRuleEvaluator(PolicyEvaluationContext ctx,
                                           std::unique_ptr<IFileSystemWrapper> fileSystemWrapper,
                                           CommandExecFunc commandExecFunc)
    : RuleEvaluator(std::move(ctx), std::move(fileSystemWrapper))
    , m_commandExecFunc(commandExecFunc
                        ? std::move(commandExecFunc)
                        : [](const std::string& cmd) { const auto cmdOutput = Utils::Exec(cmd); return std::make_pair(cmdOutput.StdOut, cmdOutput.StdErr); })
{
}

RuleResult CommandRuleEvaluator::Evaluate()
{
    LogDebug("Processing command rule: '{}'", m_ctx.rule);

    auto result = RuleResult::NotFound;

    if (m_ctx.pattern)
    {
        auto [output, error] = m_commandExecFunc(m_ctx.rule);

        // Trim ending lines if any (command output may have trailing newlines)
        output = Utils::Trim(output, "\n");
        error = Utils::Trim(error, "\n");

        if (IsRegexOrNumericPattern(*m_ctx.pattern))
        {
            const auto outputPatternMatch = sca::PatternMatches(output, *m_ctx.pattern);
            const auto errorPatternMatch = sca::PatternMatches(error, *m_ctx.pattern);

            if (outputPatternMatch || errorPatternMatch)
            {
                result = outputPatternMatch.value_or(false) || errorPatternMatch.value_or(false) ? RuleResult::Found
                                                                                                 : RuleResult::NotFound;
            }
            else
            {
                return RuleResult::Invalid;
            }
        }
        else if (output == m_ctx.pattern.value() || error == m_ctx.pattern.value())
        {
            result = RuleResult::Found;
        }
    }
    else
    {
        LogDebug("No pattern provided for command rule evaluation");
        return RuleResult::Invalid;
    }

    LogDebug("Command rule '{}' pattern '{}' {} found",
             m_ctx.rule,
             *m_ctx.pattern,
             result == RuleResult::Found ? "was" : "was not");

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
    LogDebug("Processing directory rule: '{}'", m_ctx.rule);

    if (m_ctx.pattern)
    {
        return CheckDirectoryForContents();
    }
    return CheckDirectoryExistence();
}

RuleResult DirRuleEvaluator::CheckDirectoryForContents()
{
    if (!m_fileSystemWrapper->exists(m_ctx.rule) || !m_fileSystemWrapper->is_directory(m_ctx.rule))
    {
        LogDebug("Directory '{}' does not exist or is not a directory", m_ctx.rule);
        return RuleResult::Invalid;
    }

    const auto pattern = *m_ctx.pattern; // NOLINT(bugprone-unchecked-optional-access)
    auto result = RuleResult::NotFound;

    if (IsRegexPattern(pattern))
    {
        const auto files = m_fileSystemWrapper->list_directory(m_ctx.rule);

        auto hadValue = false;

        for (const auto& file : files)
        {
            if (const auto patternMatch = sca::PatternMatches(file.filename().string(), pattern))
            {
                hadValue = true;

                if (patternMatch.value())
                {
                    result = RuleResult::Found;
                    break;
                }
            }
        }

        if (!hadValue)
        {
            // We assume that if all calls to PatternMatches return nullopt, the pattern is invalid or an error occurred
            return RuleResult::Invalid;
        }
    }
    else if (const auto content = sca::GetPattern(pattern))
    {
        const auto fileName = pattern.substr(0, pattern.find(" -> "));
        const auto files = m_fileSystemWrapper->list_directory(m_ctx.rule);

        for (const auto& file : files)
        {
            if (file.filename().string() == fileName)
            {
                result = RuleResult::NotFound;

                m_fileUtils->readLineByLine(file,
                                            [&content, &result](const std::string& line)
                                            {
                                                if (line == content.value())
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
        const auto files = m_fileSystemWrapper->list_directory(m_ctx.rule);

        for (const auto& file : files)
        {
            if (file.filename().string() == pattern)
            {
                result = RuleResult::Found;
                break;
            }
        }
    }
    LogDebug("Pattern '{}' {} found in directory '{}'",
             pattern,
             result == RuleResult::Found ? "was" : "was not",
             m_ctx.rule);

    return m_ctx.isNegated ? (result == RuleResult::Found ? RuleResult::NotFound : RuleResult::Found) : result;
}

RuleResult DirRuleEvaluator::CheckDirectoryExistence()
{
    auto result = RuleResult::NotFound;

    if (!m_fileSystemWrapper->exists(m_ctx.rule) || !m_fileSystemWrapper->is_directory(m_ctx.rule))
    {
        LogDebug("Directory '{}' does not exist or is not a directory", m_ctx.rule);
    }
    else
    {
        LogDebug("Directory '{}' exists", m_ctx.rule);
        result = RuleResult::Found;
    }

    return m_ctx.isNegated ? (result == RuleResult::Found ? RuleResult::NotFound : RuleResult::Found) : result;
}

ProcessRuleEvaluator::ProcessRuleEvaluator(PolicyEvaluationContext ctx,
                                           std::unique_ptr<IFileSystemWrapper> fileSystemWrapper,
                                           std::unique_ptr<ISysInfo> sysInfo,
                                           GetProcessesFunc getProcesses)
    : RuleEvaluator(std::move(ctx), std::move(fileSystemWrapper))
    , m_sysInfo(std::move(sysInfo))
    , m_getProcesses(getProcesses ? std::move(getProcesses) : [this]()
                     {
                         std::vector<std::string> processNames;

                         m_sysInfo->processes(
                             [&processNames](nlohmann::json& procJson)
                             {
                                 if (procJson.contains("name") && procJson["name"].is_string())
                                 {
                                     processNames.emplace_back(procJson["name"]);
                                 }
                             });

                         return processNames;
                     })
{
}

RuleResult ProcessRuleEvaluator::Evaluate()
{
    LogDebug("Processing process rule: '{}'", m_ctx.rule);

    const auto processes = m_getProcesses();
    auto result = RuleResult::NotFound;

    for (const auto& process : processes)
    {
        if (process == m_ctx.rule)
        {
            result = RuleResult::Found;
            break;
        }
    }

    LogDebug("Process '{}' {} found", m_ctx.rule, result == RuleResult::Found ? "was" : "was not");
    return m_ctx.isNegated ? (result == RuleResult::Found ? RuleResult::NotFound : RuleResult::Found) : result;
}

std::unique_ptr<IRuleEvaluator>
RuleEvaluatorFactory::CreateEvaluator(const std::string& input,
                                      std::unique_ptr<IFileSystemWrapper> fileSystemWrapper,
                                      std::unique_ptr<IFileIOUtils> fileUtils,
                                      std::unique_ptr<ISysInfo> sysInfo)
{
    if (!fileSystemWrapper)
    {
        fileSystemWrapper = std::make_unique<file_system::FileSystemWrapper>();
    }
    if (!fileUtils)
    {
        fileUtils = std::make_unique<file_io::FileIOUtils>();
    }
    if (!sysInfo)
    {
        sysInfo = std::make_unique<SysInfo>();
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
        case sca::WM_SCA_TYPE_PROCESS:
            return std::make_unique<ProcessRuleEvaluator>(ctx, std::move(fileSystemWrapper), std::move(sysInfo));
        case sca::WM_SCA_TYPE_DIR:
            return std::make_unique<DirRuleEvaluator>(ctx, std::move(fileSystemWrapper), std::move(fileUtils));
        case sca::WM_SCA_TYPE_COMMAND: return std::make_unique<CommandRuleEvaluator>(ctx, std::move(fileSystemWrapper));
        default: return nullptr;
    }
}
